#!/usr/bin/env python3
"""Roda a bateria de experimentos do Trabalho 2 (CMSTP) e consolida os
resultados nas tres tabelas exigidas pela especificacao:

  1) desvio percentual da MELHOR solucao (das N execucoes) vs melhor conhecida;
  2) desvio percentual da MEDIA das N execucoes vs melhor conhecida;
  3) tempo medio das N execucoes.

Cada algoritmo e chamado 10 vezes por instancia (sementes diferentes). O
construtivo randomizado usa >= 30 iteracoes e o reativo >= 300, com blocos de
30, conforme a especificacao.

Uso tipico (no Linux, apos 'make'):
    python3 scripts/experimentos.py                      # tudo
    python3 scripts/experimentos.py --grupos tc80        # so um grupo
    python3 scripts/experimentos.py --algos guloso,reativo
    python3 scripts/experimentos.py --bin ./cmstp --seeds 10

Saidas:
    resultados_detalhado.csv   (uma linha por instancia/Q/algoritmo/alpha)
    resultados_tabelas.csv     (as 3 tabelas, prontas p/ colar na planilha)
E as tabelas tambem sao impressas na tela.
"""

import argparse
import csv
import os
import re
import statistics
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from melhores_conhecidas import MELHORES  # noqa: E402

# ---------------------------------------------------------------------------
# Definicao das instancias. inst = 1..5; o nome no arquivo e montado por 'padrao'.
# ---------------------------------------------------------------------------
GRUPOS = {
    "tc80": dict(arquivo="instancias/capmst1.txt", padrao="tc80-{i}.txt",
                 Qs=[5, 10, 20], insts=range(1, 6)),
    "te80": dict(arquivo="instancias/capmst1.txt", padrao="te80-{i}.txt",
                 Qs=[5, 10, 20], insts=range(1, 6)),
    "cm50": dict(arquivo="instancias/capmst2.txt", padrao="cm50r{i}.dat",
                 Qs=[200, 400, 800], insts=range(1, 6)),
    "cm100": dict(arquivo="instancias/capmst2.txt", padrao="cm100r{i}.dat",
                  Qs=[200, 400, 800], insts=range(1, 6)),
    "cm200": dict(arquivo="instancias/capmst2.txt", padrao="cm200r{i}.dat",
                  Qs=[200, 400, 800], insts=range(1, 6)),
}

RE_CUSTO = re.compile(r"Custo total\.+:\s*([0-9.]+)")
RE_TEMPO = re.compile(r"Tempo \(s\)\.+:\s*([0-9.]+)")


def rodar(binario, arquivo, nome, Q, algo, seed, alpha, rand_iter,
          reat_iter, reat_bloco, alphas_reat):
    """Executa o binario uma vez e devolve (custo, tempo)."""
    cmd = [binario, "-i", arquivo, "-n", nome, "-q", str(Q), "-a", algo,
           "-s", str(seed)]
    if algo == "randomizado":
        cmd += ["-al", str(alpha), "-it", str(rand_iter)]
    elif algo == "reativo":
        cmd += ["-as", alphas_reat, "-it", str(reat_iter), "-b", str(reat_bloco)]
    try:
        saida = subprocess.run(cmd, capture_output=True, text=True,
                               timeout=1200).stdout
    except subprocess.TimeoutExpired:
        print(f"  ! timeout em {nome} Q={Q} {algo}", file=sys.stderr)
        return None, None
    mc, mt = RE_CUSTO.search(saida), RE_TEMPO.search(saida)
    if not mc or not mt:
        print(f"  ! falha ao ler saida de {nome} Q={Q} {algo}", file=sys.stderr)
        print(saida, file=sys.stderr)
        return None, None
    return float(mc.group(1)), float(mt.group(1))


def desvio(valor, bk):
    return None if bk in (None, 0) else (valor - bk) / bk * 100.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="./cmstp", help="executavel (padrao ./cmstp)")
    ap.add_argument("--grupos", default=",".join(GRUPOS),
                    help="grupos separados por virgula")
    ap.add_argument("--algos", default="guloso,randomizado,reativo")
    ap.add_argument("--seeds", type=int, default=10)
    ap.add_argument("--alphas", default="0.1,0.2,0.3",
                    help="alphas do randomizado (3 valores)")
    ap.add_argument("--rand-iter", type=int, default=30)
    ap.add_argument("--reat-iter", type=int, default=300)
    ap.add_argument("--reat-bloco", type=int, default=30)
    ap.add_argument("--alphas-reat", default="0.05,0.1,0.15,0.2,0.3,0.5")
    ap.add_argument("--detalhado", default="resultados_detalhado.csv")
    ap.add_argument("--tabelas", default="resultados_tabelas.csv")
    args = ap.parse_args()

    if not (os.path.isfile(args.bin) or shutil_which(args.bin)):
        print(f"Executavel nao encontrado: {args.bin}. Compile com 'make'.",
              file=sys.stderr)
        sys.exit(1)

    grupos = [g.strip() for g in args.grupos.split(",") if g.strip()]
    algos = [a.strip() for a in args.algos.split(",") if a.strip()]
    alphas_rand = [float(a) for a in args.alphas.split(",")]
    seeds = list(range(1, args.seeds + 1))

    linhas = []  # detalhado
    for g in grupos:
        cfg = GRUPOS[g]
        for i in cfg["insts"]:
            nome = cfg["padrao"].format(i=i)
            for Q in cfg["Qs"]:
                bk = MELHORES.get((g, i, Q))
                # lista de (rotulo_algo, algo, alpha)
                execs = []
                if "guloso" in algos:
                    execs.append(("guloso", "guloso", None))
                if "randomizado" in algos:
                    for al in alphas_rand:
                        execs.append((f"rand_a{al}", "randomizado", al))
                if "reativo" in algos:
                    execs.append(("reativo", "reativo", None))

                for rotulo, algo, alpha in execs:
                    # guloso e deterministico: 1 execucao basta
                    usar_seeds = [1] if algo == "guloso" else seeds
                    custos, tempos = [], []
                    for s in usar_seeds:
                        c, t = rodar(args.bin, cfg["arquivo"], nome, Q, algo, s,
                                     alpha, args.rand_iter, args.reat_iter,
                                     args.reat_bloco, args.alphas_reat)
                        if c is not None:
                            custos.append(c)
                            tempos.append(t)
                    if not custos:
                        continue
                    melhor = min(custos)
                    media = statistics.mean(custos)
                    tempo_med = statistics.mean(tempos)
                    linhas.append(dict(
                        grupo=g, instancia=i, Q=Q, algoritmo=rotulo,
                        melhor_conhecida=bk if bk is not None else "",
                        melhor=round(melhor, 2), media=round(media, 2),
                        desvio_melhor_pct=fmt(desvio(melhor, bk)),
                        desvio_media_pct=fmt(desvio(media, bk)),
                        tempo_medio_s=round(tempo_med, 4),
                        execucoes=len(custos)))
                    print(f"{g} {nome} Q={Q:<4} {rotulo:<10} "
                          f"melhor={melhor:.0f} media={media:.1f} "
                          f"t={tempo_med:.3f}s "
                          f"devMelhor={fmt(desvio(melhor, bk))}")

    # grava detalhado
    campos = ["grupo", "instancia", "Q", "algoritmo", "melhor_conhecida",
              "melhor", "media", "desvio_melhor_pct", "desvio_media_pct",
              "tempo_medio_s", "execucoes"]
    with open(args.detalhado, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=campos)
        w.writeheader()
        w.writerows(linhas)
    print(f"\n[ok] detalhado salvo em {args.detalhado} ({len(linhas)} linhas)")

    escrever_tabelas(linhas, args.tabelas)
    print(f"[ok] tabelas consolidadas salvas em {args.tabelas}")


def escrever_tabelas(linhas, caminho):
    """Monta as 3 tabelas medias por grupo (estilo Tabela 4/6 do artigo)."""
    algos = []
    for L in linhas:
        if L["algoritmo"] not in algos:
            algos.append(L["algoritmo"])
    grupos = []
    for L in linhas:
        if L["grupo"] not in grupos:
            grupos.append(L["grupo"])

    def media_col(grupo, algo, campo):
        vals = [float(L[campo]) for L in linhas
                if L["grupo"] == grupo and L["algoritmo"] == algo
                and L[campo] not in ("", None)]
        return round(statistics.mean(vals), 3) if vals else ""

    with open(caminho, "w", newline="") as f:
        w = csv.writer(f)
        for titulo, campo in [
                ("TABELA 1 - Desvio % da MELHOR solucao vs melhor conhecida",
                 "desvio_melhor_pct"),
                ("TABELA 2 - Desvio % da MEDIA das execucoes vs melhor conhecida",
                 "desvio_media_pct"),
                ("TABELA 3 - Tempo medio (s)", "tempo_medio_s")]:
            w.writerow([titulo])
            w.writerow(["grupo"] + algos)
            for g in grupos:
                w.writerow([g] + [media_col(g, a, campo) for a in algos])
            w.writerow([])


def fmt(x):
    return "" if x is None else round(x, 3)


def shutil_which(name):
    from shutil import which
    return which(name)


if __name__ == "__main__":
    main()
