# Árvore Geradora Capacitada de Custo Mínimo (CMSTP)

Trabalho 2 — DCC059 (Teoria dos Grafos) — UFJF
Tema **C: Capacitated Minimum Spanning Tree Problem**.
Alunos: Thomás Causin, Caio Cunha, Igor Reis

Implementação em C++ de três heurísticas construtivas para o CMSTP: um algoritmo
guloso, um guloso randomizado (GRASP) e um guloso randomizado reativo.

---

## 1. O problema

Dado um grafo não direcionado conexo `G = (V, E)`, com custos `c_e ≥ 0` nas
arestas, um vértice raiz `r` (depósito), demandas `d_i > 0` nos demais vértices e
uma capacidade `Q > 0`, queremos uma **árvore geradora enraizada em `r` de custo
mínimo** tal que, ao remover a raiz, cada sub-árvore resultante tenha demanda
total no máximo `Q`:

```
min  Σ c_e            sujeito a   Σ d_i ≤ Q   para cada sub-árvore.
   e∈E_T                        i∈S
```

Cada sub-árvore pendurada diretamente na raiz é chamada de *s-tree*. O acréscimo
da restrição de capacidade torna o problema NP-difícil, o que justifica o uso de
heurísticas.

---

## 2. Estrutura do código

```
src/
  Grafo.h / Grafo.cpp          TAD Grafo (matriz de custos + demandas) e leitor
  Solucao.h / Solucao.cpp      Árvore enraizada (vetor de pais), custo e viabilidade
  Resolvedor.h / Resolvedor.cpp Os três algoritmos
  main.cpp                     Linha de comando, semente, cronometragem e log CSV
instancias/                    Instâncias de exemplo
Makefile
```

### TAD Grafo
Como as instâncias do CMSTP são grafos completos, o grafo é armazenado em uma
**matriz de custos** `n × n`, com um vetor de **demandas** por vértice (o grafo é
ponderado nas arestas *e* nos vértices). O vértice de índice `0` é sempre a raiz.

### Solução
A árvore é representada por um **vetor de pais**: `pai[v]` é o predecessor de `v`
no caminho até a raiz. Essa representação garante que cada vértice tenha um único
pai (portanto a estrutura é sempre uma árvore) e facilita:
- recalcular o custo (`Σ c(v, pai[v])`);
- verificar viabilidade — subindo de cada vértice até a sua *sub-raiz* e somando
  as demandas por componente para comparar com `Q`.

---

## 3. Os algoritmos

Os três algoritmos partem da mesma construção baseada na heurística de
**Esau–Williams**, tradicional para o CMSTP.

### Ideia da construção (Esau–Williams)
1. Cada terminal começa ligado diretamente à raiz (cada um é uma componente).
2. Para cada vértice `i`, procura-se o vértice `j` mais próximo em **outra**
   componente cuja união **não viole a capacidade**. A *economia* de fazer essa
   união é

   ```
   economia(i) = (custo da ligação da componente de i até a raiz) − c(i, j)
   ```

   O primeiro termo é o "portão" (*gate*) da componente: a aresta mais barata
   entre a componente e a raiz.
3. Une-se o par com maior economia positiva, funde as componentes e atualiza o
   portão da componente resultante (o mais barato entre os dois).
4. Repete-se enquanto existir união com economia positiva.

Ao final, as arestas escolhidas nas uniões, mais a ligação de cada componente à
raiz pelo seu portão, formam a árvore. Uma BFS a partir da raiz orienta o vetor
de pais. A capacidade nunca é violada porque uniões inviáveis são descartadas
durante a construção.

### Guloso
Escolhe sempre a união de **maior economia** (desempate determinístico por menor
custo de aresta e menores índices). É determinístico: para a mesma instância,
sempre produz a mesma solução.

### Guloso randomizado (GRASP)
Em vez de escolher sempre o melhor candidato, monta uma **Lista Restrita de
Candidatos (RCL)** com todos os candidatos cuja economia é

```
economia ≥ economia_max − α · (economia_max − economia_min)
```

e sorteia um deles. O parâmetro `α ∈ [0, 1]` controla a aleatoriedade:
`α = 0` recai no guloso; `α = 1` sorteia entre todos os candidatos. A construção
é repetida `iteracoes` vezes e a melhor solução é retornada.

### Guloso randomizado reativo (GRASP reativo)
Trabalha com um **conjunto de valores de `α`**, cada um com uma probabilidade de
ser sorteado. As probabilidades começam uniformes e são reajustadas a cada
**bloco** de iterações, favorecendo os valores de `α` que vêm gerando soluções
de menor custo médio. A regra de atualização (Prais & Ribeiro) é

```
q_k = (melhor_custo / custo_medio_k) ^ δ         p_k = q_k / Σ q
```

com `δ = 10`. Assim, valores de `α` com melhor desempenho ganham peso ao longo da
execução, sem descartar completamente os demais.

---

## 4. Compilação

Ambiente Linux com `g++` (padrão C++17):

```bash
make
```

Gera o executável `cmstp`. Para limpar:

```bash
make clean
```

---

## 5. Formato das instâncias

O leitor (`Grafo::lerInstancia`) reconhece **dois formatos automaticamente**,
decidindo pelo primeiro token do arquivo (número → simples; texto → OR-Library).

### 5.1. Formato nativo da OR-Library (recomendado)

As instâncias clássicas do CMSTP (`tc`, `te`, `td`, `cm`) estão na OR-Library:
<http://people.brunel.ac.uk/~mastjjb/jeb/orlib/capmstinfo.html>
(arquivos `capmst1.txt` = tc/te; `capmst2.txt` = cm). O código **lê esses
arquivos diretamente**, sem conversão. Características tratadas:

- cada arquivo traz **várias instâncias emendadas**; use `-n, --nome` para
  escolher qual ler (ex.: `-n tc80-1.txt`, `-n cm50r1.dat`);
- a matriz de custos é **quadrada e inclui o depósito como último nó** (a
  dimensão é inferida automaticamente, valendo tanto para `(n+1)×(n+1)` das `tc`
  quanto para `N×N` das `cm`). Ao ler, o depósito é **remapeado para o índice 0**;
- a **diagonal** (`0` ou `1000`, conforme o arquivo) é ignorada;
- nas `cm`, as **demandas** vêm num bloco separado cujo rótulo começa por `priz`
  e são associadas automaticamente; nas demais, as demandas são **unitárias**.

Em `instancias/` já estão os arquivos `capmst1.txt` e `capmst2.txt` baixados.

### 5.2. Formato simples (próprio)

Também aceito, útil para instâncias pequenas de teste. Arquivo texto com
**apenas números**:

```
n                         (número de vértices, incluindo a raiz)
<matriz de custos n × n>  (n linhas com n valores cada)
[opcional] n demandas     (uma por vértice)
```

- O vértice `0` é a raiz; sua demanda é sempre tratada como `0`.
- Sem demandas → **unitárias**; com demandas → **não unitárias**.

Em `instancias/` há dois exemplos nesse formato: `exemplo.txt` (unitária) e
`exemplo_nao_unitario.txt` (não unitária).

### 5.3. Capacidade

A capacidade `Q` **não** faz parte de nenhum dos formatos: é passada na linha de
comando (`-q`), pois na literatura o mesmo grafo é testado com diferentes `Q`.

---

## 6. Execução

Forma geral:

```
./cmstp -i <instancia> -q <capacidade> -a <algoritmo> [opcoes]
```

| Opção | Descrição |
|-------|-----------|
| `-i`, `--instancia`  | arquivo da instância (obrigatório) |
| `-n`, `--nome`       | nome da instância dentro de um arquivo OR-Library com várias (ex.: `tc80-1.txt`) |
| `-q`, `--capacidade` | capacidade `Q` (obrigatório) |
| `-a`, `--algoritmo`  | `guloso`, `randomizado` ou `reativo` (obrigatório) |
| `-al`, `--alpha`     | `α` do randomizado (padrão `0.3`) |
| `-it`, `--iter`      | nº de iterações construtivas (padrão `30` / `300`) |
| `-b`, `--bloco`      | tamanho do bloco do reativo (padrão `30`) |
| `-as`, `--alphas`    | lista de `α` do reativo, ex: `0.1,0.2,0.3` |
| `-s`, `--semente`    | semente da randomização |
| `-o`, `--saida`      | arquivo para gravar a solução |
| `-c`, `--csv`        | arquivo CSV de resultados (padrão `resultados.csv`) |

Exemplos:

```bash
# Guloso (exemplo simples)
./cmstp -i instancias/exemplo.txt -q 5 -a guloso

# Instância real da OR-Library (escolhendo pelo nome)
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a guloso
./cmstp -i instancias/capmst2.txt -n cm50r1.dat -q 200 -a reativo -it 300 -b 30

# Randomizado (alpha 0.2, 100 iterações, semente fixa)
./cmstp -i instancias/capmst1.txt -n te80-1.txt -q 5 -a randomizado -al 0.2 -it 100 -s 12345
```

> Sem `-n`, se o arquivo tiver várias instâncias, o programa lista os nomes
> disponíveis e pede que você escolha uma.

### Semente
A semente de randomização é gerada **uma única vez** (a partir de data/hora) e
impressa na saída, para permitir repetir um teste. Também pode ser fornecida por
`-s`. Fixada a semente, a execução é reprodutível.

### Saída
No terminal são exibidos o custo total, o número de sub-árvores e a viabilidade.
Com `-o`, a solução também é gravada em arquivo, incluindo uma **lista de arestas
`u v`** que pode ser colada diretamente no editor
<http://csacademy.com/app/graph-editor/> para visualização.

### Log CSV
Ao final de cada execução, uma linha é anexada ao CSV com: data/hora, instância,
capacidade, algoritmo, parâmetros (`α`, iterações, bloco), melhor `α` (reativo),
semente, tempo em segundos e valor da melhor solução. O cabeçalho é criado
automaticamente na primeira execução.

---

## 7. Experimentos

De acordo com a especificação:
- executar cada algoritmo **10 vezes** por instância, cada vez com uma semente
  diferente, registrando a melhor solução e o tempo médio;
- no randomizado, o construtivo deve ser chamado **pelo menos 30 vezes** (`-it 30`
  ou mais); no reativo, **pelo menos 300 vezes** (`-it 300`), com blocos de 30 a
  50 iterações;
- escolher **três valores de `α`** para os testes do randomizado.

Para automatizar tudo, use o script pronto em `scripts/`:

```bash
make                                   # compila ./cmstp
python3 scripts/experimentos.py        # roda a bateria completa
# ou um subconjunto:
python3 scripts/experimentos.py --grupos tc80,cm50 --algos guloso,reativo
```

Ele roda cada algoritmo **10 vezes** por instância (sementes diferentes),
compara com as melhores soluções conhecidas (`scripts/melhores_conhecidas.py`,
extraídas do artigo do Tema C) e gera:

- `resultados_detalhado.csv` — uma linha por instância/Q/algoritmo;
- `resultados_tabelas.csv` — as **3 tabelas** exigidas (desvio da melhor, desvio
  da média e tempo médio), prontas para colar na planilha do Classroom.

> No Windows, se o `./cmstp` reclamar de biblioteca, compile estático
> (`g++ -std=c++17 -O2 -static -o cmstp.exe src/*.cpp`) e rode com
> `--bin ./cmstp.exe`.
