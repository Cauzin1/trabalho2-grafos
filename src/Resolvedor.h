#ifndef RESOLVEDOR_H
#define RESOLVEDOR_H

#include <random>
#include <vector>

#include "Grafo.h"
#include "Solucao.h"

// Reune as tres heuristicas construtivas para o CMSTP. Todas partem da mesma
// construcao no estilo Esau-Williams; o parametro alpha controla a
// aleatorizacao (GRASP).
class Resolvedor {
public:
    explicit Resolvedor(const Grafo& grafo) : g(grafo) {}

    Solucao guloso() const;
    Solucao randomizado(double alpha, int iteracoes, std::mt19937& rng) const;
    Solucao reativo(const std::vector<double>& alphas, int iteracoes,
                    int tamBloco, std::mt19937& rng, double& melhorAlpha) const;

private:
    const Grafo& g;

    // Constroi uma solucao. Com rng == nullptr a escolha e sempre a melhor
    // (guloso puro); caso contrario usa lista restrita de candidatos (RCL).
    Solucao construcao(double alpha, std::mt19937* rng) const;
};

#endif
