#ifndef SOLUCAO_H
#define SOLUCAO_H

#include <ostream>
#include <string>
#include <vector>

#include "Grafo.h"

// Arvore geradora enraizada representada por um vetor de pais:
// pai[v] indica o predecessor de v no caminho ate a raiz. A raiz aponta
// para si mesma. Cada subarvore pendurada na raiz e uma s-tree.
class Solucao {
public:
    Solucao();
    Solucao(int n, int raiz);

    void definirPai(int v, int pai) { paiDe[v] = pai; }
    int pai(int v) const { return paiDe[v]; }

    double custoTotal() const { return custo; }
    void recalcularCusto(const Grafo& g);

    bool viavel(const Grafo& g) const;
    int numeroSubarvores() const;

    void imprimir(std::ostream& os, const Grafo& g) const;
    void imprimirArestas(std::ostream& os) const;
    bool salvar(const std::string& caminho, const Grafo& g) const;

private:
    int n;
    int raiz;
    std::vector<int> paiDe;
    double custo;
};

#endif
