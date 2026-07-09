# Preparação para a Apresentação — Trabalho 2 (CMSTP / Tema C)

Guia completo para você chegar seguro na apresentação: **teoria**, **código**,
**demonstração ao vivo** e um **banco de perguntas** com respostas. Leia uma vez
com calma e treine a demo pelo menos uma vez antes.

> Lembrete: a apresentação é no **laboratório (Linux)** e o código será
> **executado na hora**. A dica número 1 é: **compile e rode antes** (Seção 8).

---

## 1. Roteiro sugerido (o que falar, em ordem)

| Etapa | Conteúdo | Tempo |
|-------|----------|-------|
| 1 | O problema (CMST): o que é, aplicações, por que é difícil | ~2 min |
| 2 | Estruturas de dados (Grafo, Solução) | ~2 min |
| 3 | A construção Esau–Williams (base dos 3 algoritmos) | ~3 min |
| 4 | Guloso → Randomizado (GRASP) → Reativo | ~3 min |
| 5 | Demonstração ao vivo (compilar + rodar os 3) | ~4 min |
| 6 | Resultados e discussão (tabelas, desvios) | ~2 min |
| 7 | Perguntas | — |

Todos os integrantes devem saber **todos** os aspectos (a nota é individual).

---

## 2. Parte teórica — os conceitos que você precisa dominar

### 2.1. O que é o CMST
**Capacitated Minimum Spanning Tree** = Árvore Geradora de Custo Mínimo com
Capacidade. Temos:
- um **vértice central / raiz** (vértice 0, o "depósito" ou processador central);
- vários **terminais**, cada um com uma **demanda** `d_i` (quanto de tráfego gera);
- **custos** `c_ij` em cada aresta (ligar `i` a `j`);
- uma **capacidade** `Q`.

**Objetivo:** conectar todos os terminais à raiz formando uma **árvore de custo
mínimo**, com a restrição de que cada **ramo que sai da raiz** (chamado de
**s-tree** ou sub-árvore) tenha **soma de demandas ≤ Q**.

**Aplicação clássica:** projeto de **redes de telecomunicações** — a central e
os terminais, com limite de tráfego por linha. Também aparece em logística e
roteamento de veículos.

### 2.2. Por que é difícil (NP-difícil)
- A **AGM comum** (sem capacidade) é resolvida em tempo **polinomial** por Prim
  ou Kruskal — são algoritmos gulosos exatos.
- **Adicionar a restrição de capacidade torna o problema NP-difícil**
  (Papadimitriou, 1978): não se conhece algoritmo que ache o ótimo em tempo
  polinomial. O número de árvores viáveis explode com `n`.
- Por isso usamos **heurísticas**: métodos que acham **boas** soluções
  **rápido**, sem garantir otimalidade. É exatamente o que a disciplina pede
  neste trabalho.

### 2.3. Heurística construtiva: Esau–Williams
É a heurística clássica do CMST e a **base dos nossos três algoritmos**.

**Ideia:** começar com todo terminal ligado direto à raiz (caro) e ir
**fundindo** terminais em ramos, sempre que isso **economizar** custo e **não
estourar** `Q`.

**Passo a passo:**
1. Cada terminal é uma componente ligada à raiz. O custo dessa ligação é o
   **portão (gate)** da componente.
2. Para cada vértice `i`, ache o `j` **mais próximo** em **outra** componente
   cuja união **caiba** em `Q`.
3. Calcule a **economia**:
   ```
   economia(i) = custo_do_portão(componente de i) − custo(i, j)
   ```
   (vale a pena se ligar em `j` sai mais barato que o portão atual até a raiz).
4. Faça a união de **maior economia positiva**, funda as componentes e mantenha
   o **portão mais barato** dos dois.
5. Repita enquanto houver economia positiva.

No fim: arestas das uniões + portões até a raiz = a árvore. Uma **BFS** a partir
da raiz define o "pai" de cada vértice. **A viabilidade é garantida por
construção** (uniões inviáveis são descartadas no passo 2).

### 2.4. GRASP (guloso randomizado)
O guloso puro sempre dá a **mesma** solução. Para explorar alternativas,
randomizamos a escolha com o parâmetro **α ∈ [0,1]**:
- monta-se uma **Lista Restrita de Candidatos (RCL)** — os candidatos "bons o
  suficiente":
  ```
  economia ≥ economia_max − α·(economia_max − economia_min)
  ```
- **sorteia-se** um da RCL.
- `α = 0` → só o melhor entra → guloso puro. `α = 1` → todos entram → aleatório.
- repete-se a construção `iterações` vezes e guarda-se a **melhor** solução.

### 2.5. GRASP reativo
Resolve "qual α usar?": trabalha com um **conjunto** de α's e **aprende** quais
funcionam melhor. As probabilidades começam iguais e, a cada **bloco** de
iterações, são reajustadas pela regra de **Prais & Ribeiro**:
```
q_k = (melhor_custo / custo_médio_do_α_k) ^ δ        p_k = q_k / Σ q      (δ=10)
```
α's que produzem médias melhores ganham mais probabilidade. É **adaptativo**.

---

## 3. Parte de implementação — o código

### 3.1. Arquitetura (4 peças)
- **`Grafo`** — TAD do grafo: matriz de custos `n×n` + vetor de demandas; lê a
  instância do arquivo.
- **`Solucao`** — a árvore-solução (vetor de pais); calcula custo e viabilidade.
- **`Resolvedor`** — os três algoritmos (compartilham a construção).
- **`main`** — linha de comando, semente, cronômetro, log CSV.

### 3.2. TAD Grafo
As instâncias do CMST são **grafos completos** → usamos **matriz de custos**
(acesso `O(1)` a qualquer `c_ij`, que é o que a construção mais faz). É
**ponderado nas arestas** (custos) **e nos vértices** (demandas). A raiz é o
índice 0.

### 3.3. Solução — vetor de pais
`pai[v]` = vértice acima de `v` no caminho até a raiz. Vantagens:
- **sempre é uma árvore** (cada vértice tem 1 pai);
- **custo** = `Σ custo(v, pai[v])`;
- **viabilidade**: para cada vértice, sobe pelos pais até a sub-raiz e soma
  demandas por ramo; se algum ramo > `Q`, é inviável.

### 3.4. Union-find na construção
Para saber rapidamente a que componente um vértice pertence e para fundir
componentes, usamos **union-find** (`comp[]`), com `demComp[]` (demanda da
componente) e `gateCusto[]/gateNo[]` (portão). Isso deixa a construção eficiente.

### 3.5. Leitura das instâncias (formato da literatura)
O leitor entende **direto** o formato da **OR-Library** (`capmst1.txt` = tc/te;
`capmst2.txt` = cm): vários problemas por arquivo (escolhe-se com `-n`), matriz
que **inclui o depósito como último nó** (remapeado para o índice 0), diagonal
ignorada e demandas do bloco `priz` (nas `cm`). Também aceita um formato simples
próprio para testes.

### 3.6. Semente, saída e CSV
- **Semente**: gerada **uma vez** (data/hora), **impressa** e reprodutível com
  `-s`.
- **Saída**: custo, nº de sub-árvores, viabilidade e uma **lista de arestas** que
  cola no `csacademy.com/app/graph-editor` para visualizar.
- **CSV**: uma linha por execução com todos os parâmetros e resultados.

### 3.7. Complexidade (se perguntarem)
Cada construção é `O(n³)` no pior caso (a cada uma das `≈ n` fusões, varre-se
`O(n²)` pares para achar o melhor candidato). O randomizado multiplica por
`iterações`; o reativo idem. Para as instâncias do trabalho (até ~200 vértices)
roda em **frações de segundo a poucos segundos**.

---

## 4. Demonstração ao vivo — comandos exatos

> Faça na ordem abaixo. Antes, **compile**: `make` (gera `./cmstp`).

**(a) Guloso — determinístico:**
```bash
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a guloso
```
Fale: "é determinístico, sempre dá o mesmo custo; repare que a solução é viável".
Rode duas vezes para mostrar que o custo não muda.

**(b) Randomizado — com semente (reprodutível):**
```bash
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a randomizado -al 0.2 -it 30 -s 42
```
Fale: "com α=0.2 ele explora; fixando a semente `-s 42`, o resultado se repete".
Rode de novo com `-s 42` (igual) e sem `-s` (muda) para mostrar o efeito.

**(c) Reativo — aprende o melhor α:**
```bash
./cmstp -i instancias/capmst2.txt -n cm50r1.dat -q 200 -a reativo -it 300 -b 30 -s 42
```
Fale: "300 construções, blocos de 30; no fim ele informa o **melhor α**; note que
esta é uma `cm`, com **demandas não unitárias**".

**(d) Visualizar a árvore:**
```bash
./cmstp -i instancias/capmst1.txt -n tc80-1.txt -q 5 -a guloso -o solucao.txt
```
Abra `solucao.txt`, copie a lista de arestas e cole em
<http://csacademy.com/app/graph-editor/> para mostrar a árvore desenhada.

**(e) Mostrar o log CSV:**
```bash
cat resultados.csv
```
Fale: "cada execução acrescenta uma linha — é o que usamos para montar as tabelas
do relatório".

---

## 5. Resultados e discussão (tenha na ponta da língua)

**Validação contra os ótimos conhecidos** (heurística construtiva, sem busca
local — reativo, 300 iterações):

| Instância | Q | Ótimo | Nosso | Desvio |
|-----------|---|-------|-------|--------|
| tc80-1 | 5 | 1099 | 1153 | 4,9% |
| te80-1 | 5 | 2544 | 2571 | 1,1% |
| cm50r1 | 200 | 1098 | 1111 | 1,2% |
| cm50r1 | 800 | 495 | 511 | 3,2% |

**Dois pontos que a banca gosta de explorar:**
1. **Por que o desvio cai quando Q aumenta?** Com `Q` maior há mais liberdade de
   fusão e menos "amarras" de capacidade; a construção gulosa chega mais perto do
   ótimo. É coerente com o artigo do Tema C.
2. **Por que o randomizado às vezes fica pior que o guloso?** Porque nossos
   algoritmos são **só construtivos** (sem busca local). A randomização explora,
   mas sem refinar; em algumas instâncias o guloso puro já é bom. O **reativo**
   tende a corrigir isso porque inclui α's pequenos (próximos do guloso).

---

## 6. Banco de perguntas prováveis (com respostas curtas)

**P: Por que não usar Prim ou Kruskal?**
R: Eles resolvem a AGM **sem** capacidade. A restrição de capacidade torna o
problema NP-difícil; Prim/Kruskal não a respeitam. Por isso usamos uma heurística
(Esau–Williams) que considera a capacidade a cada fusão.

**P: O que garante que a solução é viável (respeita Q)?**
R: Na construção, uma fusão só é considerada se `demanda(comp_i)+demanda(comp_j) ≤ Q`.
Fusões inviáveis são descartadas → nenhum ramo estoura `Q`. Ainda há
`Solucao::viavel()` que confere ao final.

**P: O que é a "economia" (tradeoff) do Esau–Williams?**
R: `economia(i) = custo do portão da componente de i − custo(i, j)`. É quanto se
poupa trocando a ligação atual até a raiz por uma ligação barata a um vizinho `j`.

**P: O que é o α?**
R: Controla o tamanho da RCL (aleatoriedade). α=0 → guloso; α=1 → aleatório;
valores intermediários equilibram explorar × seguir o melhor.

**P: Diferença entre randomizado e reativo?**
R: O randomizado usa **um** α fixo. O reativo usa **vários** α's e **ajusta as
probabilidades** ao longo da execução (Prais & Ribeiro), favorecendo os que dão
melhores médias.

**P: Como escolheram os α's?**
R: Para o randomizado testamos 3 valores (ex.: 0.1, 0.2, 0.3). Para o reativo um
conjunto maior (0.05…0.5) para o mecanismo ter opções para aprender.

**P: Por que δ = 10 na regra do reativo?**
R: É a "sensibilidade" da atualização: quanto maior o δ, mais o mecanismo
concentra probabilidade nos melhores α's. 10 dá um ajuste forte mas estável.

**P: Por que matriz de custos e não lista de adjacência?**
R: As instâncias são grafos **completos**; a construção consulta `c_ij` o tempo
todo. Matriz dá acesso `O(1)`. Lista de adjacência não traria vantagem aqui.

**P: Como representam a árvore?**
R: Por um **vetor de pais** (`pai[v]`). Garante que é sempre uma árvore e facilita
custo e checagem de viabilidade.

**P: Como a semente garante reprodutibilidade?**
R: Ela é gerada uma vez (data/hora), impressa, e alimenta o gerador `mt19937`.
Passando a mesma semente com `-s`, toda a sequência aleatória se repete → mesmo
resultado.

**P: E se Q for menor que a maior demanda?**
R: O programa detecta e recusa a instância (seria inviável — um terminal sozinho
já estouraria `Q`).

**P: Qual a complexidade?**
R: `O(n³)` por construção (≈`n` fusões × `O(n²)` para achar o melhor par). Rápido
para as instâncias do trabalho.

**P: O reativo sempre melhora o guloso?**
R: Não necessariamente em toda instância, mas na média tende a ser melhor ou
igual, porque explora e se adapta. Sem busca local, os ganhos são moderados.

**P: Qual a limitação do trabalho / como melhorar?**
R: Não há **busca local** após a construção. Adicionar movimentos (mover vértice
de ramo, trocar dois vértices) recomputando o custo reduziria bastante os
desvios — é o que o artigo do Tema C faz com o BRKGA.

---

## 7. Sugestão de divisão entre os 3 integrantes

Como a nota é individual e todos devem conhecer tudo, apenas **quem conduz** cada
parte muda:
- **Integrante A:** problema + estruturas de dados (Seções 2.1–2.2, 3.1–3.4).
- **Integrante B:** os três algoritmos (Seções 2.3–2.5) + demonstração ao vivo.
- **Integrante C:** leitura das instâncias, semente/CSV, resultados e discussão
  (Seções 3.5–3.7, 5).

Mesmo assim, **todos treinem responder as perguntas da Seção 6**.

---

## 8. Checklist pré-apresentação (faça na véspera E no lab)

- [ ] `make clean && make` compila **sem erros** na máquina do laboratório.
- [ ] Rodou o **guloso**, o **randomizado** e o **reativo** ao menos uma vez.
- [ ] Testou os arquivos `instancias/capmst1.txt` e `capmst2.txt` estão no repo
      clonado (não esqueça de dar `git pull`).
- [ ] Abriu uma solução no **csacademy** para garantir que a visualização funciona.
- [ ] Confere que o **CSV** é criado/append corretamente.
- [ ] Cada integrante consegue explicar Esau–Williams **sem ler**.
- [ ] Tem à mão os **números de validação** (Seção 5) para citar.
- [ ] Sabe dizer **o que falta** (busca local) — mostra maturidade.

> Se por acaso der problema de biblioteca ao rodar no Windows, compile com
> `g++ -std=c++17 -O2 -static -o cmstp src/*.cpp`. **No Linux do laboratório
> basta `make`.**

---

## 9. Frases de segurança (se travar)

- "Nosso trabalho implementa três heurísticas construtivas para o CMST, todas
  baseadas na fusão de componentes do Esau–Williams, respeitando a capacidade."
- "A randomização entra pela RCL controlada por α; o reativo aprende os melhores
  α's ao longo da execução."
- "A viabilidade é garantida por construção e reconferida ao final."
- "Validamos comparando com as melhores soluções conhecidas do artigo do tema."
</content>
