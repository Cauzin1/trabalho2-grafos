#include "Grafo.h"

#include <fstream>
#include <iostream>

Grafo::Grafo() : n(0), raiz(0), Q(0.0), naoUnitarias(false) {}

double Grafo::maiorDemanda() const {
    double m = 0.0;
    for (int i = 0; i < n; ++i)
        if (i != raiz && demandas[i] > m) m = demandas[i];
    return m;
}

// Formato aceito (apenas numeros, separados por espaco/quebra de linha):
//   n
//   matriz de custos n x n
//   [opcional] n demandas (uma por vertice)
// Se as demandas nao forem informadas, assume-se demanda unitaria para todos
// os terminais. O vertice 0 e sempre a raiz e sua demanda e forcada para 0.
bool Grafo::lerInstancia(const std::string& caminho) {
    std::ifstream in(caminho);
    if (!in) {
        std::cerr << "Nao foi possivel abrir a instancia: " << caminho << "\n";
        return false;
    }

    if (!(in >> n) || n <= 1) {
        std::cerr << "Numero de vertices invalido na instancia.\n";
        return false;
    }

    std::vector<double> valores;
    double x;
    while (in >> x) valores.push_back(x);

    const size_t esperadoMatriz = static_cast<size_t>(n) * n;
    if (valores.size() < esperadoMatriz) {
        std::cerr << "Instancia incompleta: esperava " << esperadoMatriz
                  << " custos e encontrei " << valores.size() << ".\n";
        return false;
    }

    matrizCusto.assign(n, std::vector<double>(n, 0.0));
    size_t k = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            matrizCusto[i][j] = valores[k++];

    demandas.assign(n, 1.0);
    demandas[raiz] = 0.0;

    if (valores.size() - esperadoMatriz >= static_cast<size_t>(n)) {
        for (int i = 0; i < n; ++i) demandas[i] = valores[k++];
        demandas[raiz] = 0.0;
        for (int i = 0; i < n; ++i)
            if (i != raiz && demandas[i] != 1.0) naoUnitarias = true;
    }

    return true;
}
