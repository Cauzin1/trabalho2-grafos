#include "Resolvedor.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>

namespace {
constexpr double INF = std::numeric_limits<double>::infinity();
constexpr double EPS = 1e-9;
constexpr double DELTA = 10.0;  // sensibilidade da atualizacao reativa
}  // namespace

// Esau-Williams: parte de todos ligados a raiz e funde componentes pela maior
// economia (gate da componente de i menos custo da aresta i-j) sem violar Q.
Solucao Resolvedor::construcao(double alpha, std::mt19937* rng) const {
    const int n = g.getNumVertices();
    const int raiz = g.getRaiz();
    const double Q = g.getCapacidade();

    std::vector<int> comp(n);            // union-find
    std::vector<double> demComp(n, 0.0); // demanda acumulada da componente
    std::vector<double> gateCusto(n, 0.0); // menor custo componente -> raiz
    std::vector<int> gateNo(n, 0);         // vertice que faz essa ligacao

    for (int v = 0; v < n; ++v) {
        comp[v] = v;
        if (v == raiz) continue;
        demComp[v] = g.demanda(v);
        gateCusto[v] = g.custo(raiz, v);
        gateNo[v] = v;
    }

    std::function<int(int)> encontrar = [&](int x) {
        while (comp[x] != x) { comp[x] = comp[comp[x]]; x = comp[x]; }
        return x;
    };

    struct Candidato { double economia; double custoAresta; int i; int j; };
    auto melhorQue = [](const Candidato& a, const Candidato& b) {
        if (std::abs(a.economia - b.economia) > EPS) return a.economia > b.economia;
        if (std::abs(a.custoAresta - b.custoAresta) > EPS) return a.custoAresta < b.custoAresta;
        if (a.i != b.i) return a.i < b.i;
        return a.j < b.j;
    };

    std::vector<std::pair<int, int>> arestasInternas;

    while (true) {
        std::vector<Candidato> candidatos;
        candidatos.reserve(n);

        for (int i = 0; i < n; ++i) {
            if (i == raiz) continue;
            int ci = encontrar(i);
            int melhorJ = -1;
            double melhorCusto = INF;
            for (int j = 0; j < n; ++j) {
                if (j == raiz || j == i) continue;
                int cj = encontrar(j);
                if (cj == ci) continue;
                if (demComp[ci] + demComp[cj] > Q + EPS) continue;
                double c = g.custo(i, j);
                if (c < melhorCusto) { melhorCusto = c; melhorJ = j; }
            }
            if (melhorJ != -1) {
                double economia = gateCusto[ci] - melhorCusto;
                if (economia > EPS)
                    candidatos.push_back({economia, melhorCusto, i, melhorJ});
            }
        }

        if (candidatos.empty()) break;

        int escolhido = 0;
        if (rng == nullptr) {
            for (int k = 1; k < static_cast<int>(candidatos.size()); ++k)
                if (melhorQue(candidatos[k], candidatos[escolhido])) escolhido = k;
        } else {
            double eMax = -INF, eMin = INF;
            for (const auto& c : candidatos) {
                eMax = std::max(eMax, c.economia);
                eMin = std::min(eMin, c.economia);
            }
            double limiar = eMax - alpha * (eMax - eMin);
            std::vector<int> rcl;
            for (int k = 0; k < static_cast<int>(candidatos.size()); ++k)
                if (candidatos[k].economia >= limiar - EPS) rcl.push_back(k);
            std::uniform_int_distribution<int> dist(0, static_cast<int>(rcl.size()) - 1);
            escolhido = rcl[dist(*rng)];
        }

        int i = candidatos[escolhido].i;
        int j = candidatos[escolhido].j;
        int ci = encontrar(i);
        int cj = encontrar(j);
        arestasInternas.push_back({i, j});

        double novaDem = demComp[ci] + demComp[cj];
        double novoGateCusto;
        int novoGateNo;
        if (gateCusto[ci] <= gateCusto[cj]) {
            novoGateCusto = gateCusto[ci];
            novoGateNo = gateNo[ci];
        } else {
            novoGateCusto = gateCusto[cj];
            novoGateNo = gateNo[cj];
        }

        comp[ci] = cj;  // cj passa a ser a raiz da componente unida
        demComp[cj] = novaDem;
        gateCusto[cj] = novoGateCusto;
        gateNo[cj] = novoGateNo;
    }

    // Monta a arvore (arestas internas + gates) e orienta com BFS da raiz.
    std::vector<std::vector<int>> adj(n);
    for (const auto& e : arestasInternas) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    std::vector<char> tratada(n, 0);
    for (int v = 0; v < n; ++v) {
        if (v == raiz) continue;
        int cr = encontrar(v);
        if (!tratada[cr]) {
            tratada[cr] = 1;
            int gate = gateNo[cr];
            adj[raiz].push_back(gate);
            adj[gate].push_back(raiz);
        }
    }

    Solucao sol(n, raiz);
    std::vector<char> visitado(n, 0);
    std::queue<int> fila;
    fila.push(raiz);
    visitado[raiz] = 1;
    sol.definirPai(raiz, raiz);
    while (!fila.empty()) {
        int u = fila.front();
        fila.pop();
        for (int w : adj[u])
            if (!visitado[w]) {
                visitado[w] = 1;
                sol.definirPai(w, u);
                fila.push(w);
            }
    }
    sol.recalcularCusto(g);
    return sol;
}

Solucao Resolvedor::guloso() const {
    return construcao(0.0, nullptr);
}

Solucao Resolvedor::randomizado(double alpha, int iteracoes, std::mt19937& rng) const {
    Solucao melhor;
    double melhorCusto = INF;
    for (int it = 0; it < iteracoes; ++it) {
        Solucao s = construcao(alpha, &rng);
        if (s.custoTotal() < melhorCusto) {
            melhorCusto = s.custoTotal();
            melhor = s;
        }
    }
    return melhor;
}

// GRASP reativo: reajusta as probabilidades dos alphas a cada bloco, favorecendo
// os de menor custo medio (Prais & Ribeiro).
Solucao Resolvedor::reativo(const std::vector<double>& alphas, int iteracoes,
                            int tamBloco, std::mt19937& rng,
                            double& melhorAlpha) const {
    const int m = static_cast<int>(alphas.size());
    std::vector<double> prob(m, 1.0 / m);
    std::vector<double> somaCusto(m, 0.0);
    std::vector<int> contagem(m, 0);

    Solucao melhor;
    double melhorCusto = INF;
    melhorAlpha = alphas[0];

    std::uniform_real_distribution<double> uni(0.0, 1.0);

    for (int it = 1; it <= iteracoes; ++it) {
        double r = uni(rng);
        double acumulado = 0.0;
        int idx = m - 1;
        for (int k = 0; k < m; ++k) {
            acumulado += prob[k];
            if (r <= acumulado) { idx = k; break; }
        }

        Solucao s = construcao(alphas[idx], &rng);
        double c = s.custoTotal();
        somaCusto[idx] += c;
        contagem[idx] += 1;
        if (c < melhorCusto) {
            melhorCusto = c;
            melhor = s;
            melhorAlpha = alphas[idx];
        }

        if (it % tamBloco == 0) {
            std::vector<double> q(m);
            double soma = 0.0;
            for (int k = 0; k < m; ++k) {
                if (contagem[k] == 0) {
                    q[k] = 1.0;
                } else {
                    double media = somaCusto[k] / contagem[k];
                    q[k] = std::pow(melhorCusto / media, DELTA);
                }
                soma += q[k];
            }
            for (int k = 0; k < m; ++k) prob[k] = q[k] / soma;
        }
    }
    return melhor;
}
