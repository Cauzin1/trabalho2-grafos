# Compila o projeto no Windows (PowerShell), sem precisar de 'make'.
# Uso:  .\compilar.ps1
# Gera:  cmstp.exe
#
# O -static evita conflito de DLL do libstdc++ no Windows (que causava segfault).
# No laboratorio (Linux) use 'make' normalmente.

g++ -std=c++17 -O2 -Wall -Wextra -static -o cmstp.exe `
    src\main.cpp src\Grafo.cpp src\Solucao.cpp src\Resolvedor.cpp

if ($LASTEXITCODE -eq 0) {
    Write-Output "OK: cmstp.exe compilado."
    Write-Output "Exemplo: .\cmstp.exe -i instancias\capmst1.txt -n tc80-1.txt -q 5 -a guloso"
} else {
    Write-Output "ERRO na compilacao (codigo $LASTEXITCODE)."
}
