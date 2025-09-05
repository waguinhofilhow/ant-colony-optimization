import os
import csv
import matplotlib.pyplot as plt
import numpy as np

# Caminho para a pasta com os arquivos CSV
pasta = "resultados_tempos"  # ajuste para o seu caminho

medias = []
desvios = []
nomes_arquivos = []

# Itera sobre todos os arquivos .csv da pasta
for nome_arquivo in os.listdir(pasta):
    if nome_arquivo.endswith(".csv"):
        caminho_completo = os.path.join(pasta, nome_arquivo)
        
        with open(caminho_completo, "r") as f:
            leitor = csv.reader(f)
            tempos = [float(linha[0]) for linha in leitor if linha]

        if tempos:
            media = np.mean(tempos)
            desvio = np.std(tempos)
            medias.append(media)
            desvios.append(desvio)
            nomes_arquivos.append(nome_arquivo.replace(".csv", ""))

# Geração do gráfico
x = np.arange(len(nomes_arquivos))
plt.figure(figsize=(12, 6))
plt.bar(x, medias, yerr=desvios, capsize=5)
plt.xticks(x, nomes_arquivos, rotation=45, ha="right")
plt.ylabel("Tempo de Execução (s)")
plt.title("Tempo médio de execução por configuração")
plt.tight_layout()
plt.grid(True, axis="y")
plt.show()