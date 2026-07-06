#ifndef GRAFO_H
#define GRAFO_H

#include <string>
#include <vector>

// Grafo nao direcionado, ponderado nas arestas (custos) e nos vertices (demandas).
// A instancia do CMSTP e um grafo completo, entao a estrutura interna e uma
// matriz de custos. O vertice de indice 0 e a raiz/deposito.
class Grafo {
public:
    Grafo();

    bool lerInstancia(const std::string& caminho);

    int getNumVertices() const { return n; }
    int getRaiz() const { return raiz; }
    double getCapacidade() const { return Q; }
    void setCapacidade(double q) { Q = q; }

    double custo(int i, int j) const { return matrizCusto[i][j]; }
    double demanda(int i) const { return demandas[i]; }

    double maiorDemanda() const;
    bool temDemandasNaoUnitarias() const { return naoUnitarias; }

private:
    int n;
    int raiz;
    double Q;
    bool naoUnitarias;
    std::vector<std::vector<double>> matrizCusto;
    std::vector<double> demandas;
};

#endif
