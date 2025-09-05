import numpy as np
import matplotlib.pyplot as plt

# Caminho do arquivo CSV
arquivo = 'iteracoes_x_melhor_custo.csv'

# Carrega os dados do CSV (assumindo separação por vírgula, sem cabeçalho)
dados = np.loadtxt(arquivo, delimiter=',')

# Separa colunas: x = iterações, y = custo
iteracoes = dados[:, 0]
custos = dados[:, 1]

# Cria o gráfico
plt.figure(figsize=(8, 5))
plt.plot(iteracoes, custos, marker='o', linestyle='-', color='blue', label='Melhor custo')
plt.xlabel('Iteração')
plt.ylabel('Melhor custo')
plt.title('Evolução do melhor custo por iteração')
plt.grid(True)
plt.legend()

# Exibe o gráfico
plt.tight_layout()
plt.show()

# Opcional: salvar como imagem
# plt.savefig('grafico_evolucao_custo.png')