# Guia De Revisão — Trabalho 2 (CMSTP / Tema C)

Este documento explica, de forma passo a passo e em linguagem acessível, **o que
o problema pede**, **como o código funciona** e **o que ainda falta entregar**.
Serve tanto para você entender cada parte quanto de base para escrever o
relatório. Ele **não substitui** o relatório (que tem modelo próprio no
Classroom e não pode conter código-fonte).

---

## 1. Que problema estamos resolvendo?

O tema é o **Capacitated Minimum Spanning Tree Problem (CMSTP)** — Árvore
Geradora Capacitada de Custo Mínimo.

Imagine um **processador central** (a raiz, vértice `0`) e vários **terminais**
(os demais vértices). Precisamos ligar todos os terminais à central formando uma
**árvore** (sem ciclos, tudo conectado) e gastando o **menor custo total de
arestas** possível.

A diferença para uma Árvore Geradora Mínima (AGM/MST) comum é a **restrição de
capacidade**: cada "ramo" que sai diretamente da raiz — chamado de **s-tree** ou
**sub-árvore** — só pode acumular uma demanda total até `Q`. Cada terminal `i`
tem uma demanda `d_i` (quanto de tráfego ele gera). A soma das demandas de todos
os terminais dentro de um mesmo ramo não pode passar de `Q`.

```
              (0) raiz
             / | \
           A   B   C        <- cada ramo A, B, C é uma s-tree
          /|   |   |\
         ...  ...  ...       demanda(A) ≤ Q, demanda(B) ≤ Q, demanda(C) ≤ Q
```

**Por que isso é difícil?** A AGM comum resolve-se em tempo polinomial (Prim,
Kruskal). Mas **adicionar a capacidade torna o problema NP-difícil** — não se
conhece algoritmo eficiente que garanta a solução ótima. Por isso usamos
**heurísticas**: métodos que encontram boas soluções rapidamente, sem garantia
de otimalidade. É exatamente o objetivo do trabalho.

**Modelo matemático (resumo):**
```
minimizar   Σ  custo(aresta)         (soma dos custos das arestas usadas)
sujeito a   cada s-tree tem Σ demandas ≤ Q
            o resultado é uma árvore enraizada em 0
```

---

## 2. Visão geral do código

O código está dividido em quatro peças, cada uma com uma responsabilidade clara:

| Arquivo | Papel |
|---------|-------|
| `Grafo.h/.cpp` | **TAD Grafo** — guarda a matriz de custos e as demandas; lê a instância do arquivo |
| `Solucao.h/.cpp` | Representa **uma árvore-solução**; calcula custo e verifica viabilidade |
| `Resolvedor.h/.cpp` | Os **três algoritmos** (guloso, randomizado, reativo) |
| `main.cpp` | Linha de comando, semente aleatória, cronômetro e log CSV |

O fluxo de uma execução é:

```
main lê os argumentos
   └─ Grafo.lerInstancia(arquivo)         (monta matriz de custos + demandas)
   └─ gera/le a semente aleatória
   └─ Resolvedor.<algoritmo>()            (constrói a solução)
   └─ Solucao.imprimir() / salvar()        (mostra e grava)
   └─ registrarCsv()                       (anexa 1 linha no .csv)
```

---

## 3. O TAD Grafo (`Grafo.h/.cpp`)

**Decisão de projeto:** as instâncias clássicas do CMSTP são **grafos completos**
(existe aresta entre todo par de vértices). Então, em vez de listas de
adjacência, guardamos uma **matriz de custos `n × n`** (`matrizCusto[i][j]` = custo
da aresta entre `i` e `j`) e um **vetor de demandas** (`demandas[i]`).

Isso atende ao que a especificação pede: um grafo **ponderado nas arestas** (os
custos) **e nos vértices** (as demandas). O vértice `0` é sempre a raiz e sua
demanda é forçada a `0`.

**Formato de leitura** (`lerInstancia`): o leitor reconhece **dois formatos**
automaticamente, olhando o primeiro token do arquivo.

1. **Formato nativo da OR-Library** (quando o arquivo começa com um nome de
   texto, ex.: `tc80-1.txt`). É o formato *padrão da literatura* — os arquivos
   `capmst1.txt` (tc/te) e `capmst2.txt` (cm) são lidos **diretamente**. O leitor:
   - separa as **várias instâncias emendadas** no mesmo arquivo (escolha com `-n`);
   - lê a matriz quadrada que **inclui o depósito como último nó** (a dimensão é
     descoberta pela raiz quadrada da quantidade de custos) e **remapeia o
     depósito para o índice 0**;
   - ignora a **diagonal** (0 ou 1000);
   - nas `cm`, pega as **demandas** do bloco `priz` separado; nas demais, unitárias.

2. **Formato simples próprio** (quando começa com um número):
   ```
   n                         <- número de vértices (inclui a raiz)
   matriz n × n de custos
   [opcional] n demandas     <- se ausente, terminais recebem demanda 1
   ```
   Útil para instâncias pequenas de teste (`exemplo.txt`).

A capacidade `Q` **não** está no arquivo — vem pela linha de comando (`-q`),
porque na literatura o mesmo grafo é testado com vários `Q`.

---

## 4. A Solução (`Solucao.h/.cpp`)

**Como representamos uma árvore?** Por um **vetor de pais** (`paiDe`):
`paiDe[v]` é o vértice imediatamente acima de `v` no caminho até a raiz. A raiz
aponta para si mesma.

Essa representação é elegante porque:
- **Garante que sempre é uma árvore**: cada vértice tem exatamente um pai, então
  não há como criar dois caminhos até a raiz.
- **Custo é trivial de calcular**: `custo = Σ custo(v, paiDe[v])` para todo `v`
  que não é a raiz (`recalcularCusto`).
- **Viabilidade é fácil de checar** (`viavel`): para cada vértice, "sobe" pelos
  pais até achar a **sub-raiz** (o filho direto da raiz) e acumula a demanda por
  ramo. Se algum ramo passar de `Q`, a solução é inviável.

```cpp
// coração da verificação de viabilidade:
int s = v;
while (paiDe[s] != raiz) s = paiDe[s];   // sobe até a sub-raiz
demandaPorSubarvore[s] += g.demanda(v);  // acumula no ramo
```

Também há `imprimirArestas`, que escreve uma aresta por linha (`u v`) no formato
que o **csacademy.com/app/graph-editor** aceita — permite visualizar a árvore
copiando e colando.

---

## 5. A construção Esau–Williams (o coração dos três algoritmos)

Os três algoritmos partem da **mesma ideia de construção**, a heurística clássica
de **Esau–Williams** para o CMSTP. Vale entender bem, porque é o centro do
trabalho.

**Passo a passo:**

1. **Início:** cada terminal está ligado **diretamente à raiz**. Ou seja, cada
   terminal é sua própria "componente" (um ramo de um vértice só). O custo de
   ligar a componente à raiz é a aresta `custo(raiz, i)` — chamamos isso de
   **portão (gate)** da componente.

2. **Buscar uniões vantajosas:** para cada vértice `i`, procuramos o vértice `j`
   **mais próximo** que esteja em **outra componente**, desde que a união
   **não estoure a capacidade** (`demanda(comp_i) + demanda(comp_j) ≤ Q`).

3. **Calcular a economia** de fazer essa união:
   ```
   economia(i) = (custo do portão da componente de i) − custo(i, j)
   ```
   Interpretação: se ligar `i` a `j` (barato) sai mais em conta do que manter o
   portão atual até a raiz, vale a pena — economizamos a diferença.

4. **Escolher e unir:** pega-se o par com **maior economia positiva**, adiciona a
   aresta `(i, j)`, funde as duas componentes e o novo portão passa a ser o
   **mais barato** entre os dois portões antigos.

5. **Repetir** enquanto existir alguma união com economia positiva.

No final, as arestas escolhidas nas uniões + a ligação de cada componente à raiz
(pelo seu portão) formam a árvore. Uma **BFS a partir da raiz** orienta tudo,
preenchendo o vetor de pais. **A capacidade nunca é violada** porque uniões
inviáveis são descartadas já no passo 2.

**Detalhes de implementação que valem citar:**
- Usa-se **union-find** (`comp[]`) para saber rapidamente a qual componente cada
  vértice pertence e para fundi-las.
- `demComp[]` guarda a demanda acumulada de cada componente (para checar `Q`).
- `gateCusto[]` e `gateNo[]` guardam o custo e o vértice do portão de cada
  componente.

> A escolha de comparar apenas o portão da componente de `i` (e não o do `j`) é a
> formulação tradicional do Esau–Williams vista em sala. Ela **nunca gera solução
> inviável**; no máximo, escolhe uma ordem de união ligeiramente diferente. Uma
> variação possível seria usar `max(portão_i, portão_j)` na economia (veja Seção 11).

---

## 6. Algoritmo Guloso

O guloso é a construção acima **sem nenhuma aleatoriedade**: em cada passo escolhe
sempre a união de **maior economia**. Em caso de empate, desempata de forma
determinística (menor custo de aresta, depois menores índices).

**Consequência:** é **determinístico** — para a mesma instância e o mesmo `Q`,
produz sempre exatamente a mesma solução. No código, isso corresponde a chamar a
construção com `rng == nullptr`.

```cpp
Solucao Resolvedor::guloso() const { return construcao(0.0, nullptr); }
```

---

## 7. Guloso Randomizado (GRASP)

Aqui entra a aleatoriedade controlada pelo parâmetro **α ∈ [0, 1]**.

Em vez de pegar sempre o melhor candidato, montamos uma **Lista Restrita de
Candidatos (RCL)** — os candidatos "bons o suficiente":
```
economia ≥ economia_max − α · (economia_max − economia_min)
```
e **sorteamos** um deles.

Intuição do α:
- **α = 0** → só o melhor entra na RCL → vira o **guloso puro**.
- **α = 1** → todos os candidatos entram → escolha **totalmente aleatória**.
- valores intermediários → equilíbrio entre "seguir o guloso" e "explorar".

Como cada construção pode dar uma árvore diferente, repetimos a construção
`iteracoes` vezes (parâmetro `-it`) e **guardamos a melhor** solução encontrada.

```cpp
for (int it = 0; it < iteracoes; ++it) {
    Solucao s = construcao(alpha, &rng);
    if (s.custoTotal() < melhorCusto) { melhorCusto = s.custoTotal(); melhor = s; }
}
```

> **Observação importante para o relatório:** como não há busca local depois da
> construção, é **normal** o randomizado às vezes ficar **um pouco pior** que o
> guloso em uma instância (foi o que aconteceu em `grande80` com α=0,2). Isso é
> esperado em GRASP "só construtivo" e é um bom ponto de discussão.

---

## 8. Guloso Randomizado Reativo (GRASP reativo)

O reativo resolve uma pergunta prática: **qual α usar?** Em vez de fixar um,
ele trabalha com um **conjunto de valores de α** e **aprende** quais funcionam
melhor durante a execução.

Como funciona:
1. Cada α começa com a **mesma probabilidade** de ser sorteado.
2. A cada iteração, sorteia-se um α (conforme as probabilidades atuais), constrói
   uma solução e registra o custo obtido por aquele α.
3. A cada **bloco** de iterações (parâmetro `-b`, tipicamente 30–50), as
   probabilidades são **reajustadas** pela regra de **Prais & Ribeiro**:
   ```
   q_k = (melhor_custo / custo_médio_do_α_k) ^ δ        p_k = q_k / Σ q
   ```
   com `δ = 10`. Como `custo_médio ≥ melhor_custo`, a razão fica em (0, 1]; α's
   que produziram **médias melhores** (mais próximas do melhor) ganham `q` maior
   e, portanto, **mais probabilidade**. Os piores perdem peso, mas não somem.

Assim o algoritmo **se auto-ajusta**, concentrando o esforço nos α's mais
promissores. Ao final, reporta qual α produziu a melhor solução (`melhorAlpha`).

Parâmetros exigidos pela especificação (todos presentes): **conjunto de α**
(`-as`), **número de iterações** (`-it`) e **tamanho do bloco** (`-b`).

---

## 9. Infraestrutura: semente, saída e CSV (`main.cpp`)

**Semente de randomização** (exigência da especificação):
- É gerada **uma única vez**, a partir do relógio de alta resolução
  (`high_resolution_clock`), quando não é informada.
- É **impressa** na tela (`Semente........:`), para você poder repetir um teste.
- Pode ser **informada** com `-s <n>`. Com a semente fixa, a execução é 100%
  **reprodutível**.

**Saída da solução:** no terminal mostra custo total, número de sub-árvores e
viabilidade. Com `-o arquivo`, grava também a **lista de arestas** para o
csacademy.

**Log CSV** (`resultados.csv` por padrão): a cada execução, anexa **uma linha**
com data/hora, instância, capacidade, algoritmo, parâmetros (α, iterações,
bloco), melhor α (reativo), semente, tempo (s) e melhor solução. O cabeçalho é
criado automaticamente na primeira vez. Colunas que não se aplicam a um algoritmo
ficam vazias — exatamente a "tabela única" sugerida pela especificação.

---

## 10. Como compilar e executar

**Compilar** (Linux, C++17):
```bash
make            # gera o executável ./cmstp
make clean      # limpa
```

**Executar** (forma geral):
```bash
./cmstp -i <instancia> -q <capacidade> -a <algoritmo> [opções]
```

Exemplos (instância real da OR-Library, escolhida por `-n`):
```bash
# Guloso
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a guloso

# Randomizado (α=0.2, 30 iterações, semente fixa)
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a randomizado -al 0.2 -it 30 -s 12345

# Reativo (300 iterações, blocos de 30) numa cm (demandas não unitárias)
./cmstp -i instancias/capmst2.txt -n cm50r1.dat -q 200 -a reativo -as 0.1,0.2,0.3 -it 300 -b 30
```

**Rodar os 10 testes de uma instância** (variando a semente):
```bash
for s in 1 2 3 4 5 6 7 8 9 10; do
  ./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a randomizado -al 0.2 -it 30 -s $s
done
# cada execução acrescenta uma linha em resultados.csv
```

---

## 11. Checklist da especificação — o que está pronto ✅ e o que falta ⚠️

**Implementação (código):**
- ✅ TAD Grafo (matriz de custos + demandas; ponderado em arestas e vértices)
- ✅ Semente por data/hora, chamada uma única vez, impressa e parametrizável (`-s`)
- ✅ Leitura de instância de arquivo **no formato nativo da literatura**
  (OR-Library `capmst1`/`capmst2`), além do formato simples próprio
- ✅ Algoritmo guloso
- ✅ Guloso randomizado com α e nº de iterações como parâmetros
- ✅ Guloso randomizado reativo com α's, iterações e tamanho de bloco como parâmetros
- ✅ Impressão da solução em formato visualizável (csacademy)
- ✅ Log em `.csv` com todos os campos sugeridos
- ✅ `.txt` (COMO_EXECUTAR.txt) com instruções de linha de comando para Linux
- ✅ Compila e executa (testado: compila sem warnings; roda os três algoritmos)

**O que ainda precisa ser feito (entregáveis fora do código):**
- ⚠️ **Relatório** no modelo do Classroom (até 10 páginas, **sem** código-fonte),
  com as **3 tabelas obrigatórias**:
  1. Desvio percentual da **melhor** solução (das 10 execuções) vs. melhor
     solução conhecida, por algoritmo e instância;
  2. Desvio percentual da **média** das 10 execuções vs. melhor conhecida;
  3. **Tempo médio** das 10 execuções.
- ✅ **Instâncias reais** já estão em `instancias/` (`capmst1.txt` = tc/te,
  `capmst2.txt` = cm) e são lidas diretamente. Faltam as `td` (vêm do artigo do
  Martins, não estão na OR-Library). As melhores soluções conhecidas estão nas
  Tabelas 9–11 do artigo do Tema C (use-as como referência do desvio percentual).
- ⚠️ **Rodar o experimento completo:** 10 execuções por instância (sementes
  diferentes); randomizado com **≥ 30** construções; reativo com **≥ 300**
  construções e blocos de 30–50. O código já respeita esses mínimos por padrão.
- ⚠️ **Escolher e justificar 3 valores de α** para os testes do randomizado.
- ⚠️ Preencher a **planilha de desvio percentual** fornecida no Classroom.

---

## 12. Pontos de atenção e possíveis melhorias

Coisas que **não são erros**, mas valem discussão/decisão:

1. **Formato de leitura — RESOLVIDO.** O leitor agora entende o formato nativo
   da OR-Library (`capmst1`/`capmst2`) direto, incluindo a seleção da instância
   por nome (`-n`), o depósito como último nó e as demandas do bloco `priz`.
   Validação contra os ótimos conhecidos (heurística construtiva, sem busca
   local): tc80-1 Q=5 → 1153 vs 1099 (4,9%); te80-1 Q=5 → 2571 vs 2544 (1,1%);
   cm50r1 Q=200 → 1111 vs 1098 (1,2%). O gap diminui com Q maior (cm50r1 Q=800 →
   511 vs 495, 3,2%), como o artigo descreve — sinal de que a leitura está certa.

2. **Ausência de busca local.** Os três algoritmos são *só construtivos*. Não é
   exigido pelo enunciado, mas explica por que o randomizado às vezes não supera
   o guloso. Se quiser resultados melhores, uma busca local simples (ex.: mover
   um vértice de um ramo para outro se reduzir custo e respeitar `Q`) ajudaria
   bastante — e renderia boa discussão no relatório.

3. **Fórmula da economia no Esau–Williams.** Usa o portão da componente de `i`.
   Uma variante usa `max(portão_i, portão_j) − custo(i,j)`, que reflete melhor a
   economia real de uma fusão. Trocar isso pode mudar (levemente) a qualidade.

4. **Robustez com `-it 0`.** Se alguém passar `-it 0` no randomizado, retorna uma
   solução vazia. Basta garantir no mínimo 1 iteração (defensivo).

5. **Ambiente de execução.** No Linux com `g++` compila e roda direto. Em
   máquinas Windows com múltiplos toolchains (MinGW + Git), pode haver conflito
   de DLL do `libstdc++`; nesse caso, compilar com `-static` resolve. **No
   laboratório (Linux) isso não ocorre.**

---

## 13. Resumo de uma frase por conceito (para revisar antes da apresentação)

- **CMSTP:** AGM com limite de demanda por ramo → NP-difícil → heurísticas.
- **TAD Grafo:** matriz de custos + vetor de demandas (grafo completo, ponderado
  em arestas e vértices).
- **Solução:** vetor de pais → sempre uma árvore; custo e viabilidade fáceis.
- **Esau–Williams:** começa tudo na raiz e vai fundindo componentes pela maior
  economia, sem estourar `Q`.
- **Guloso:** Esau–Williams sem sorteio (determinístico).
- **Randomizado (GRASP):** monta RCL com α, sorteia, repete N vezes, guarda a melhor.
- **Reativo:** aprende quais α's são melhores e ajusta as probabilidades por bloco
  (Prais & Ribeiro).
- **Semente:** uma vez, impressa, reprodutível com `-s`.
</content>
</invoke>
