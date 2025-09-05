import os
import numpy as np
import matplotlib.pyplot as plt

# Caminho da pasta contendo os arquivos .csv
pasta = 'resultados_tempo_melhor_custo'
arquivos = sorted([f for f in os.listdir(pasta) if f.endswith('.csv')])

tempos_medios = []
tempos_std = []
melhores_solucoes = []

for arquivo in arquivos:
    caminho = os.path.join(pasta, arquivo)
    dados = np.loadtxt(caminho, delimiter=',')
    
    tempos = dados[:, 0]
    solucoes = dados[:, 1]
    
    tempos_medios.append(np.mean(tempos))
    tempos_std.append(np.std(tempos))
    melhores_solucoes.append(np.min(solucoes))

# Índices dos experimentos
x = np.arange(len(arquivos))

# Separar os grupos
x_rho = x[:10]
x_ants = x[10:]

tempos_rho = tempos_medios[:10]
tempos_std_rho = tempos_std[:10]
solucoes_rho = melhores_solucoes[:10]

tempos_ants = tempos_medios[10:]
tempos_std_ants = tempos_std[10:]
solucoes_ants = melhores_solucoes[10:]

# Criar o gráfico
fig, ax1 = plt.subplots(figsize=(10, 6))

# Plot dos tempos médios com erro
ax1.errorbar(x_rho, tempos_rho, yerr=tempos_std_rho, fmt='o-', color='blue', label='Tempo médio (rho)')
ax1.errorbar(x_ants, tempos_ants, yerr=tempos_std_ants, fmt='s-', color='green', label='Tempo médio (numAnts)')
ax1.set_xlabel('Experimento')
ax1.set_ylabel('Tempo médio (ms)')
ax1.tick_params(axis='y', labelcolor='black')

# Segundo eixo Y para a melhor solução
ax2 = ax1.twinx()
ax2.plot(x_rho, solucoes_rho, 'o--', color='red', label='Melhor solução (rho)')
ax2.plot(x_ants, solucoes_ants, 's--', color='orange', label='Melhor solução (numAnts)')
ax2.set_ylabel('Melhor solução encontrada')
ax2.tick_params(axis='y', labelcolor='black')

# Título e legendas
plt.title('Tempo médio e melhor solução por experimento')
fig.tight_layout()
fig.legend(loc='upper center', bbox_to_anchor=(0.5, 1.1), ncol=2)
plt.grid(True)
plt.show()
