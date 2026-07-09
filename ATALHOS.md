# Atalhos — Comandos rápidos pra apresentação

Meu colinha pessoal. Só copiar e colar. As diferenças por sistema são basicamente:
- **Linux/Mac**: separador `/`, executável `./cmstp`
- **Windows**: separador `\`, executável `.\cmstp.exe`

---

## 0. Antes de tudo — compilar

### Linux / Mac
```bash
make
```
Se não tiver `make` no Mac, ou der erro:
```bash
g++ -std=c++17 -O2 -o cmstp src/main.cpp src/Grafo.cpp src/Solucao.cpp src/Resolvedor.cpp
```

### Windows (PowerShell)
```powershell
.\compilar.ps1
```
Ou o comando cru:
```powershell
g++ -std=c++17 -O2 -static -o cmstp.exe src\main.cpp src\Grafo.cpp src\Solucao.cpp src\Resolvedor.cpp
```

---

## 1. Guloso (determinístico)

### Linux / Mac
```bash
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a guloso
```
### Windows
```powershell
.\cmstp.exe -i instancias\capmst1.txt -n tc80-1.txt -q 5 -a guloso
```
> Rodar 2x pra mostrar que o custo NÃO muda (é determinístico).

---

## 2. Randomizado (GRASP) — com semente fixa (reprodutível)

### Linux / Mac
```bash
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a randomizado -al 0.2 -it 30 -s 42
```
### Windows
```powershell
.\cmstp.exe -i instancias\capmst1.txt -n tc80-1.txt -q 5 -a randomizado -al 0.2 -it 30 -s 42
```
> Rodar de novo com `-s 42` (igual) e depois SEM `-s` (muda) pra mostrar o efeito da semente.

---

## 3. Reativo — instância cm (demanda não unitária)

### Linux / Mac
```bash
./cmstp -i instancias/capmst2.txt -n cm50r1.dat -q 200 -a reativo -it 300 -b 30 -s 42
```
### Windows
```powershell
.\cmstp.exe -i instancias\capmst2.txt -n cm50r1.dat -q 200 -a reativo -it 300 -b 30 -s 42
```
> Falar: 300 construções, blocos de 30, no fim mostra o **melhor alpha**.

---

## 4. Visualizar a árvore (csacademy)

### Linux / Mac
```bash
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a guloso -o solucao.txt
cat solucao.txt
```
### Windows
```powershell
.\cmstp.exe -i instancias\capmst1.txt -n tc80-1.txt -q 5 -a guloso -o solucao.txt
notepad solucao.txt
```
> Copiar a lista de arestas do fim do arquivo e colar em:
> http://csacademy.com/app/graph-editor/

---

## 5. Mostrar o log CSV

### Linux / Mac
```bash
cat resultados.csv
```
### Windows
```powershell
Get-Content resultados.csv
```

---

## 6. Rodar a bateria de experimentos (gerar as tabelas)

### Linux / Mac
```bash
python3 scripts/experimentos.py
```
### Windows
```powershell
python scripts\experimentos.py --bin .\cmstp.exe
```
> Testar rápido só um grupo:
> - Linux/Mac: `python3 scripts/experimentos.py --grupos tc80 --algos guloso,reativo`
> - Windows: `python scripts\experimentos.py --bin .\cmstp.exe --grupos tc80 --algos guloso,reativo`

Gera: `resultados_detalhado.csv` e `resultados_tabelas.csv`.

---

## 7. Parâmetros (colar rápido se perguntarem)

| Flag | O que é |
|------|---------|
| `-i` | arquivo da instância |
| `-n` | nome da instância dentro do arquivo (ex: `tc80-1.txt`, `cm50r1.dat`) |
| `-q` | capacidade Q |
| `-a` | `guloso` \| `randomizado` \| `reativo` |
| `-al`| alpha do randomizado |
| `-it`| nº de iterações |
| `-b` | tamanho do bloco (reativo) |
| `-as`| lista de alphas do reativo (ex: `0.1,0.2,0.3`) |
| `-s` | semente (reprodutibilidade) |
| `-o` | grava a solução em arquivo |

---

## 8. Nomes de instância disponíveis

- **capmst1.txt** (tc/te, demanda unitária): `tc40-1..5`, `tc80-1..5`, `te40-1..5`, `te80-1..5`
- **capmst2.txt** (cm, demanda não unitária): `cm50r1..5`, `cm100r1..5`, `cm200r1..5`
- **Q** típico: tc/te → `5, 10, 20`  |  cm → `200, 400, 800`

---

## 9. Se der ruim na hora

- **`make` não existe (Windows):** usa `.\compilar.ps1`.
- **Segfault / erro de DLL (Windows):** recompila com `-static` (o `.\compilar.ps1` já faz).
- **"O arquivo contem varias instancias":** esqueci o `-n`. Ele lista os nomes; escolhe um.
- **"Instancia inviavel":** o `-q` está menor que a maior demanda. Aumenta o Q.
- **No lab (Linux):** antes de tudo → `git pull` e depois `make`.
</content>
