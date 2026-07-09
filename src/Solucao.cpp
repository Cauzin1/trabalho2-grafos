#include "Solucao.h"

#include <fstream>
#include <map>

Solucao::Solucao() : n(0), raiz(0), custo(0.0) {}

Solucao::Solucao(int n_, int raiz_)
    : n(n_), raiz(raiz_), paiDe(n_, raiz_), custo(0.0) {}

void Solucao::recalcularCusto(const Grafo& g) {
    custo = 0.0;
    for (int v = 0; v < n; ++v)
        if (v != raiz) custo += g.custo(v, paiDe[v]);
}

int Solucao::numeroSubarvores() const {
    int total = 0;
    for (int v = 0; v < n; ++v)
        if (v != raiz && paiDe[v] == raiz) ++total;
    return total;
}

// Viavel se nenhuma s-tree (soma de demandas ate a sub-raiz) excede Q.
bool Solucao::viavel(const Grafo& g) const {
    std::map<int, double> demandaPorSubarvore;
    for (int v = 0; v < n; ++v) {
        if (v == raiz) continue;
        int s = v;
        while (paiDe[s] != raiz) s = paiDe[s];
        demandaPorSubarvore[s] += g.demanda(v);
    }
    for (const auto& par : demandaPorSubarvore)
        if (par.second > g.getCapacidade() + 1e-9) return false;
    return true;
}

void Solucao::imprimir(std::ostream& os, const Grafo& g) const {
    os << "Custo total....: " << custo << "\n";
    os << "Sub-arvores....: " << numeroSubarvores() << "\n";
    os << "Viavel.........: " << (viavel(g) ? "sim" : "nao") << "\n";
    os << "Arestas (pai -> filho):\n";
    for (int v = 0; v < n; ++v)
        if (v != raiz) os << "  " << paiDe[v] << " -> " << v << "\n";
}

// Uma aresta por linha (u v), formato do editor csacademy.
void Solucao::imprimirArestas(std::ostream& os) const {
    for (int v = 0; v < n; ++v)
        if (v != raiz) os << paiDe[v] << " " << v << "\n";
}

bool Solucao::salvar(const std::string& caminho, const Grafo& g) const {
    std::ofstream out(caminho);
    if (!out) return false;
    imprimir(out, g);
    out << "\nLista de arestas (csacademy):\n";
    imprimirArestas(out);
    return true;
}
