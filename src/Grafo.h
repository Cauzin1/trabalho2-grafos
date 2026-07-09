#ifndef GRAFO_H
#define GRAFO_H

#include <string>
#include <vector>

// Grafo completo ponderado nas arestas e nos vertices; vertice 0 e a raiz.
class Grafo {
public:
    Grafo();

    // Le a instancia do arquivo; 'nome' seleciona uma dentre varias emendadas.
    bool lerInstancia(const std::string& caminho, const std::string& nome = "");

    int getNumVertices() const { return n; }
    int getRaiz() const { return raiz; }
    double getCapacidade() const { return Q; }
    void setCapacidade(double q) { Q = q; }

    double custo(int i, int j) const { return matrizCusto[i][j]; }
    double demanda(int i) const { return demandas[i]; }

    double maiorDemanda() const;
    bool temDemandasNaoUnitarias() const { return naoUnitarias; }

private:
    bool lerFormatoSimples(const std::vector<std::string>& toks);
    bool lerFormatoOrLib(const std::vector<std::string>& toks,
                         const std::string& nome);

    int n;
    int raiz;
    double Q;
    bool naoUnitarias;
    std::vector<std::vector<double>> matrizCusto;
    std::vector<double> demandas;
};

#endif
