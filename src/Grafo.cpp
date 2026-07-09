#include "Grafo.h"

#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

Grafo::Grafo() : n(0), raiz(0), Q(0.0), naoUnitarias(false) {}

double Grafo::maiorDemanda() const {
    double m = 0.0;
    for (int i = 0; i < n; ++i)
        if (i != raiz && demandas[i] > m) m = demandas[i];
    return m;
}

namespace {

bool ehRotulo(const std::string& t) {
    for (char c : t)
        if (std::isalpha(static_cast<unsigned char>(c))) return true;
    return false;
}

bool ehSeparador(const std::string& t) {
    for (char c : t)
        if (c != '-') return false;
    return !t.empty();
}

double paraNumero(const std::string& t) { return std::stod(t); }

struct Bloco {
    std::string rotulo;
    std::vector<double> nums;
};

}  // namespace

// Detecta o formato pelo 1o token: numero -> simples; texto -> OR-Library.
bool Grafo::lerInstancia(const std::string& caminho, const std::string& nome) {
    std::ifstream in(caminho);
    if (!in) {
        std::cerr << "Nao foi possivel abrir a instancia: " << caminho << "\n";
        return false;
    }

    std::vector<std::string> toks;
    std::string t;
    while (in >> t) toks.push_back(t);
    if (toks.empty()) {
        std::cerr << "Instancia vazia.\n";
        return false;
    }

    if (!ehRotulo(toks[0])) return lerFormatoSimples(toks);
    return lerFormatoOrLib(toks, nome);
}

bool Grafo::lerFormatoSimples(const std::vector<std::string>& toks) {
    size_t p = 0;
    n = static_cast<int>(paraNumero(toks[p++]));
    if (n <= 1) {
        std::cerr << "Numero de vertices invalido na instancia.\n";
        return false;
    }

    std::vector<double> valores;
    for (; p < toks.size(); ++p) valores.push_back(paraNumero(toks[p]));

    const size_t esperadoMatriz = static_cast<size_t>(n) * n;
    if (valores.size() < esperadoMatriz) {
        std::cerr << "Instancia incompleta: esperava " << esperadoMatriz
                  << " custos e encontrei " << valores.size() << ".\n";
        return false;
    }

    matrizCusto.assign(n, std::vector<double>(n, 0.0));
    size_t k = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) matrizCusto[i][j] = valores[k++];

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

// Formato OR-Library: matriz quadrada com o deposito no ultimo no (remapeado
// para o indice 0) e, nas 'cm', demandas no bloco 'priz'; diagonal ignorada.
bool Grafo::lerFormatoOrLib(const std::vector<std::string>& toks,
                            const std::string& nome) {
    std::vector<Bloco> blocos;
    for (const std::string& tk : toks) {
        if (ehSeparador(tk)) continue;
        if (ehRotulo(tk)) {
            blocos.push_back(Bloco{tk, {}});
        } else if (!blocos.empty()) {
            blocos.back().nums.push_back(paraNumero(tk));
        }
    }

    auto ehPriz = [](const std::string& r) { return r.rfind("priz", 0) == 0; };

    const Bloco* inst = nullptr;
    std::vector<const Bloco*> candidatos;
    for (const auto& b : blocos)
        if (!ehPriz(b.rotulo)) candidatos.push_back(&b);

    if (!nome.empty()) {
        for (const Bloco* b : candidatos)
            if (b->rotulo == nome || b->rotulo.rfind(nome, 0) == 0) { inst = b; break; }
        if (!inst) {
            std::cerr << "Instancia '" << nome << "' nao encontrada no arquivo.\n";
            return false;
        }
    } else if (candidatos.size() == 1) {
        inst = candidatos.front();
    } else {
        std::cerr << "O arquivo contem varias instancias; informe qual com --nome.\n"
                     "Disponiveis:";
        for (const Bloco* b : candidatos) std::cerr << ' ' << b->rotulo;
        std::cerr << "\n";
        return false;
    }

    if (inst->nums.size() < 2) {
        std::cerr << "Instancia '" << inst->rotulo << "' sem dados suficientes.\n";
        return false;
    }
    const size_t qtd = inst->nums.size() - 1;
    const int lado = static_cast<int>(std::llround(std::sqrt(static_cast<double>(qtd))));
    if (static_cast<size_t>(lado) * lado != qtd || lado <= 1) {
        std::cerr << "Matriz de custos nao e quadrada em '" << inst->rotulo << "'.\n";
        return false;
    }

    std::vector<std::vector<double>> M(lado, std::vector<double>(lado, 0.0));
    size_t k = 1;
    for (int i = 0; i < lado; ++i)
        for (int j = 0; j < lado; ++j) M[i][j] = inst->nums[k++];

    const Bloco* pesos = nullptr;
    for (const auto& b : blocos)
        if (ehPriz(b.rotulo) && static_cast<int>(b.nums.size()) == lado - 1) {
            pesos = &b;
            break;
        }

    const int dep = lado - 1;
    auto novoIdx = [&](int velho) { return velho == dep ? 0 : velho + 1; };

    n = lado;
    raiz = 0;
    matrizCusto.assign(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < lado; ++i)
        for (int j = 0; j < lado; ++j)
            if (i != j) matrizCusto[novoIdx(i)][novoIdx(j)] = M[i][j];

    demandas.assign(n, 1.0);
    demandas[raiz] = 0.0;
    if (pesos) {
        for (int t = 0; t < dep; ++t) demandas[t + 1] = pesos->nums[t];
        for (int i = 1; i < n; ++i)
            if (demandas[i] != 1.0) naoUnitarias = true;
    }
    return true;
}
