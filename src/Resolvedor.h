#ifndef RESOLVEDOR_H
#define RESOLVEDOR_H

#include <random>
#include <vector>

#include "Grafo.h"
#include "Solucao.h"

// As tres heuristicas construtivas (Esau-Williams); alpha controla a
// aleatorizacao do GRASP.
class Resolvedor {
public:
    explicit Resolvedor(const Grafo& grafo) : g(grafo) {}

    Solucao guloso() const;
    Solucao randomizado(double alpha, int iteracoes, std::mt19937& rng) const;
    Solucao reativo(const std::vector<double>& alphas, int iteracoes,
                    int tamBloco, std::mt19937& rng, double& melhorAlpha) const;

private:
    const Grafo& g;

    // Constroi uma solucao; rng == nullptr => guloso puro, senao usa RCL.
    Solucao construcao(double alpha, std::mt19937* rng) const;
};

#endif
