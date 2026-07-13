#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Grafo.h"
#include "Resolvedor.h"
#include "Solucao.h"

namespace {

struct Config {
    std::string instancia;
    std::string nome;
    std::string algoritmo;
    std::string saida;
    std::string csv = "resultados.csv";
    double capacidade = -1.0;
    double alpha = 0.3;
    int iteracoes = -1;
    int bloco = 30;
    std::vector<double> alphas = {0.05, 0.10, 0.15, 0.20, 0.30, 0.50};
    uint64_t semente = 0;
    bool sementeInformada = false;
};

void uso(const char* prog) {
    std::cerr <<
        "Uso: " << prog << " -i <instancia> -q <capacidade> -a <algoritmo> [opcoes]\n\n"
        "Algoritmos:\n"
        "  guloso                 Esau-Williams deterministico\n"
        "  randomizado            GRASP construtivo (usa -alpha e -iter)\n"
        "  reativo                GRASP reativo (usa -alphas, -iter e -bloco)\n\n"
        "Opcoes:\n"
        "  -i,  --instancia  <arq>    arquivo da instancia\n"
        "  -n,  --nome       <nome>   instancia a ler dentro de um arquivo OR-Library\n"
        "                             com varias emendadas (ex: tc80-1.txt, cm50r1.dat)\n"
        "  -q,  --capacidade <Q>      capacidade Q das sub-arvores\n"
        "  -a,  --algoritmo  <nome>   guloso | randomizado | reativo\n"
        "  -al, --alpha      <a>      alpha do randomizado (padrao 0.3)\n"
        "  -it, --iter       <n>      iteracoes construtivas\n"
        "  -b,  --bloco      <n>      tamanho do bloco do reativo (padrao 30)\n"
        "  -as, --alphas     <lista>  lista de alphas do reativo, ex: 0.1,0.2,0.3\n"
        "  -s,  --semente    <n>      semente da randomizacao\n"
        "  -o,  --saida      <arq>    arquivo para gravar a solucao\n"
        "  -c,  --csv        <arq>    arquivo csv de resultados (padrao resultados.csv)\n";
}

std::vector<double> parseAlphas(const std::string& s) {
    std::vector<double> v;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) v.push_back(std::stod(item));
    }
    return v;
}

bool parseArgs(int argc, char** argv, Config& cfg) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto prox = [&](const char* nome) -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "Faltou valor para " << nome << "\n";
                exit(1);
            }
            return argv[++i];
        };
        if (a == "-i" || a == "--instancia") cfg.instancia = prox("instancia");
        else if (a == "-n" || a == "--nome") cfg.nome = prox("nome");
        else if (a == "-q" || a == "--capacidade") cfg.capacidade = std::stod(prox("capacidade"));
        else if (a == "-a" || a == "--algoritmo") cfg.algoritmo = prox("algoritmo");
        else if (a == "-al" || a == "--alpha") cfg.alpha = std::stod(prox("alpha"));
        else if (a == "-it" || a == "--iter") cfg.iteracoes = std::stoi(prox("iter"));
        else if (a == "-b" || a == "--bloco") cfg.bloco = std::stoi(prox("bloco"));
        else if (a == "-as" || a == "--alphas") cfg.alphas = parseAlphas(prox("alphas"));
        else if (a == "-s" || a == "--semente") { cfg.semente = std::stoull(prox("semente")); cfg.sementeInformada = true; }
        else if (a == "-o" || a == "--saida") cfg.saida = prox("saida");
        else if (a == "-c" || a == "--csv") cfg.csv = prox("csv");
        else if (a == "-h" || a == "--help") { uso(argv[0]); exit(0); }
        else { std::cerr << "Argumento desconhecido: " << a << "\n"; return false; }
    }
    if (cfg.instancia.empty() || cfg.algoritmo.empty() || cfg.capacidade < 0) {
        std::cerr << "Parametros obrigatorios: -i, -q e -a.\n\n";
        return false;
    }
    if (cfg.iteracoes < 0)
        cfg.iteracoes = (cfg.algoritmo == "reativo") ? 300 : 30;
    return true;
}

std::string dataHora() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

std::string juntar(const std::vector<double>& v) {
    std::ostringstream os;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) os << ';';
        os << v[i];
    }
    return os.str();
}

void registrarCsv(const Config& cfg, double tempo, double custo,
                  double melhorAlpha) {
    bool novo = !std::ifstream(cfg.csv).good();
    std::ofstream out(cfg.csv, std::ios::app);
    if (!out) return;
    if (novo) {
        out << "datahora,instancia,capacidade,algoritmo,alpha,iteracoes,"
               "bloco,melhor_alpha,semente,tempo_s,melhor_solucao\n";
    }
    out << dataHora() << ',' << cfg.instancia << ',' << cfg.capacidade << ','
        << cfg.algoritmo << ',';
    if (cfg.algoritmo == "randomizado") out << cfg.alpha;
    else if (cfg.algoritmo == "reativo") out << juntar(cfg.alphas);
    out << ',';
    if (cfg.algoritmo != "guloso") out << cfg.iteracoes;
    out << ',';
    if (cfg.algoritmo == "reativo") out << cfg.bloco;
    out << ',';
    if (cfg.algoritmo == "reativo") out << melhorAlpha;
    out << ',' << cfg.semente << ',' << std::fixed << std::setprecision(3)
        << tempo << ',' << std::setprecision(6) << custo << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    Config cfg;
    if (!parseArgs(argc, argv, cfg)) {
        uso(argv[0]);
        return 1;
    }

    Grafo g;
    if (!g.lerInstancia(cfg.instancia, cfg.nome)) return 1;
    g.setCapacidade(cfg.capacidade);

    if (g.maiorDemanda() > cfg.capacidade + 1e-9) {
        std::cerr << "Instancia inviavel: existe demanda maior que a capacidade Q.\n";
        return 1;
    }

    if (!cfg.sementeInformada) {
        cfg.semente = static_cast<uint64_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    std::mt19937 rng(static_cast<uint32_t>(cfg.semente));

    std::cout << "Instancia......: " << cfg.instancia << "\n";
    std::cout << "Vertices.......: " << g.getNumVertices()
              << " (raiz = " << g.getRaiz() << ")\n";
    std::cout << "Capacidade Q...: " << cfg.capacidade << "\n";
    std::cout << "Demandas.......: "
              << (g.temDemandasNaoUnitarias() ? "nao unitarias" : "unitarias") << "\n";
    std::cout << "Algoritmo......: " << cfg.algoritmo << "\n";
    std::cout << "Semente........: " << cfg.semente << "\n";

    Resolvedor resolvedor(g);
    Solucao sol;
    double melhorAlpha = -1.0;

    auto inicio = std::chrono::high_resolution_clock::now();
    if (cfg.algoritmo == "guloso") {
        sol = resolvedor.guloso();
    } else if (cfg.algoritmo == "randomizado") {
        std::cout << "Alpha..........: " << cfg.alpha << "\n";
        std::cout << "Iteracoes......: " << cfg.iteracoes << "\n";
        sol = resolvedor.randomizado(cfg.alpha, cfg.iteracoes, rng);
    } else if (cfg.algoritmo == "reativo") {
        std::cout << "Alphas.........: " << juntar(cfg.alphas) << "\n";
        std::cout << "Iteracoes......: " << cfg.iteracoes << "\n";
        std::cout << "Bloco..........: " << cfg.bloco << "\n";
        sol = resolvedor.reativo(cfg.alphas, cfg.iteracoes, cfg.bloco, rng, melhorAlpha);
    } else {
        std::cerr << "Algoritmo invalido: " << cfg.algoritmo << "\n";
        return 1;
    }
    auto fim = std::chrono::high_resolution_clock::now();
    double tempo = std::chrono::duration<double>(fim - inicio).count();

    std::cout << "-----------------------------------------\n";
    sol.imprimir(std::cout, g);
    std::cout << "\n--- CSAcademy (copie e cole) ---\n";
    sol.imprimirArestas(std::cout);
    if (melhorAlpha >= 0.0)
        std::cout << "Melhor alpha...: " << melhorAlpha << "\n";
    std::cout << "Tempo (s)......: " << std::fixed << std::setprecision(3)
              << tempo << "\n";

    if (!cfg.saida.empty()) {
        if (sol.salvar(cfg.saida))
            std::cout << "Solucao gravada em: " << cfg.saida << "\n";
        else
            std::cerr << "Falha ao gravar a solucao em: " << cfg.saida << "\n";
    }

    registrarCsv(cfg, tempo, sol.custoTotal(), melhorAlpha);
    return 0;
}
