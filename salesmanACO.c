#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct Ant_ {
    int currentCity;
    int* path;       // vetor de tamanho n
    int* visited;     // vetor de flags
    double totalCost;
} Ant;

int** ler_matriz_csv(const char* filename, int n) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    int** matriz = malloc(n * sizeof(int*));
    if (!matriz) {
        perror("Erro ao alocar memória");
        fclose(file);
        return NULL;
    }

    char linha[1024];
    int linha_atual = 0;

    while (fgets(linha, sizeof(linha), file) && linha_atual < n) {
        matriz[linha_atual] = malloc(n * sizeof(int));
        if (!matriz[linha_atual]) {
            perror("Erro ao alocar memória para linha");
            // libera as linhas já alocadas
            for (int i = 0; i < linha_atual; i++) {
                free(matriz[i]);
            }
            free(matriz);
            fclose(file);
            return NULL;
        }

        char* token = strtok(linha, ",");
        for (int i = 0; i < n; i++) {
            if (token) {
                matriz[linha_atual][i] = atoi(token);
                token = strtok(NULL, ",");
            } else {
                matriz[linha_atual][i] = 0; // preenchimento padrão se faltarem elementos
            }
        }
        linha_atual++;
    }

    fclose(file);
    return matriz;
}

int nearestNeighborCost(int** distance, int n) {
    int* visited = calloc(n, sizeof(int));
    int cost = 0;
    int currentCity = 0;  // pode ser aleatório se quiser

    visited[currentCity] = 1;

    for (int step = 1; step < n; step++) {
        int nextCity = -1;
        int minDist = 10000000;  // equivalente a infinito

        for (int j = 0; j < n; j++) {
            if (!visited[j] && distance[currentCity][j] < minDist) {
                minDist = distance[currentCity][j];
                nextCity = j;
            }
        }

        if (nextCity == -1) break;  // todas visitadas (deve terminar normalmente)

        visited[nextCity] = 1;
        cost += minDist;
        currentCity = nextCity;
    }

    // Retorna à cidade inicial
    cost += distance[currentCity][0];

    free(visited);
    return cost;
}

double** initPheromone(int n, int L_nn){
    double **pheromone = calloc(n, sizeof(double*));

    double tau0 = 1.0 / (n * L_nn);

    for(int i = 0; i < n; i++){
        pheromone[i] = calloc(n, sizeof(double));
        for(int j = 0; j < n; j++){
            if (i != j)
            pheromone[i][j] = tau0;
        else
            pheromone[i][j] = 0.0;
        }
    }

    return pheromone;
}

double** initVisibility(int n, int** distance){
    double **visibility = calloc(n, sizeof(double*));

    for(int i = 0; i < n; i++){
        visibility[i] = calloc(n, sizeof(double));
        for(int j = 0; j < n; j++){
            if (i != j)
            visibility[i][j] = 1.0/distance[i][j];
        else
            visibility[i][j] = 0.0;
        }
    }

    return visibility;
}

Ant* initAnts(int numAnts, int n){
    Ant* ants = calloc(numAnts, sizeof(Ant));
    
    for(int i = 0; i < numAnts; i++){
        ants[i].path = calloc(n, sizeof(int));
        ants[i].visited = calloc(n, sizeof(int));
        ants[i].currentCity = i%n;
        ants[i].path[0] = i%n;
        ants[i].visited[i%n] = 1;
        ants[i].totalCost = 0.0;
    }

    return ants;
}

int chooseNextCity(Ant* ant, int n, double** pheromone, double** visibility, double alpha, double beta){
    double probSum = 0.0;       //Somatório do denominador da fórmula de probabilidade
    int currentCity = ant->currentCity;

    double* prob = calloc(n, sizeof(double));

    for(int i = 0; i < n; i++){
        if(ant->visited[i]){
            prob[i] = 0;
        } else {
            prob[i] = pow(pheromone[currentCity][i], alpha) * pow(visibility[currentCity][i], beta);
            probSum += prob[i];
        }
    }

    double limit = ((double) rand() / RAND_MAX) * probSum;  //Define um número entre 0 e probSum;
    double cumulative = 0.0;

    for(int i = 0; i < n; i++){     // A cada iteração, soma a probabilidade da aresta ij. Se a soma acumulada passar do limite, escolhe a aresta.
        cumulative += prob[i];
        if(cumulative >= limit){
            free(prob);
            return i;
        }
    }

    // fallback (raro) - Escolhe a primeira cidade não visitada.
    for (int j = 0; j < n; j++) {
        if (!ant->visited[j]) {
            free(prob);
            return j;
        }
    }

    free(prob);
    return -1;
}

void salvarIteracoesxMelhorCSV(int numIter, int melhorCusto, char* nomeArquivo) {

    FILE* arquivo = fopen(nomeArquivo, "a");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fprintf(arquivo, "%d,%d\n", numIter, melhorCusto);
    fclose(arquivo);
}

double* antColony(int** distance, int n, double rho, int numAnts, double alpha, double beta, int Q, int maxIter){

    double* results = calloc(2, sizeof(double));
    
    clock_t inicio = clock();
    
    int L_nn = nearestNeighborCost(distance, n);    //Heurística para guiar a inicialização de feromônio (nearest neighbor)
    double **pheromone = initPheromone(n, L_nn);
    double **visibility = initVisibility(n, distance);
    Ant *ants = initAnts(numAnts, n);

    int bestGlobalCost = 100000000;
    int countEstagnation = 0;

    int iter;
    
    for(iter = 0; iter < maxIter; iter++){

        //Construção de caminho pelas formigas
        for(int step = 1; step < n; step++){

            for(int i = 0; i < numAnts; i++){   //Escolhe próxima cidade e atualiza cidade atual, custo, etc.
                int nextCity = chooseNextCity(&ants[i], n, pheromone, visibility, alpha, beta);

                ants[i].path[step] = nextCity;
                ants[i].visited[nextCity] = 1;
                ants[i].totalCost += distance[ants[i].currentCity][nextCity];
                ants[i].currentCity = nextCity;
            }
        }

        for(int i = 0; i < numAnts; i++){   //Quando todas as cidades tiverem sido percorridas.
            int origin = ants[i].path[0];
            ants[i].totalCost += distance[ants[i].currentCity][origin];
        }

        //Verificação do menor caminho da iteração e se há estagnação
        int bestCostOfIter = 100000000;

        for(int k = 0; k < numAnts; k++){
            if(ants[k].totalCost < bestCostOfIter){
                bestCostOfIter = ants[k].totalCost;
            }
        }

        if(bestCostOfIter < bestGlobalCost){
            countEstagnation = 0;
            bestGlobalCost = bestCostOfIter;
        } else {
            countEstagnation++;
        }

        if(countEstagnation == 100){
            break;
        }

        //Atualização de feromonios

        //Evaporação
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                pheromone[i][j] *= (1.0 - rho);
            }
        }

        //Depósito de feromônio
        for (int k = 0; k < numAnts; k++) {
            double contrib = Q / ants[k].totalCost;

            for (int i = 0; i < n - 1; i++) {
                int from = ants[k].path[i];     //Deposita apenas nas arestas percorridas.
                int to = ants[k].path[i + 1];
                pheromone[from][to] += contrib;
                pheromone[to][from] += contrib;
            }

            // Feromônio da ultima aresta (alguma cidade -> primeira cidade da formiga)
            int last = ants[k].path[n - 1];
            int first = ants[k].path[0];
            pheromone[last][first] += contrib;
            pheromone[first][last] += contrib;
        }

        //Reinicio das formigas
        for(int k = 0; k < numAnts; k++){
            for(int i = 0; i < n; i++){
                ants[k].path[i] = 0;
                ants[k].visited[i] = 0;
            }
            ants[k].currentCity = k%n;
            ants[k].path[0] = k%n;
            ants[k].visited[k%n] = 1;
            ants[k].totalCost = 0.0;
        }
        //salvarIteracoesxMelhorCSV(iter, bestGlobalCost, "iteracoes_x_melhor_custo.csv");
    }

    //printf("Melhor custo: %d\n",bestGlobalCost);

    clock_t fim = clock();
    double tempoSegundos = (double)(fim - inicio) / CLOCKS_PER_SEC;

    for(int i = 0; i < n; i++){
        free(pheromone[i]);
        free(visibility[i]);
    }
    free(pheromone);
    free(visibility);
    for(int i = 0; i < numAnts; i++){
        free(ants[i].path);
        free(ants[i].visited);
    }
    free(ants);

    results[1] = bestGlobalCost;
    results[0] = tempoSegundos*1000;

    //return iter;
    //return tempoSegundos*1000;

    return results;
}

void salvarTempoCSV(double tempo, double valorConvergencia, char* nomeArquivo) {

    FILE* arquivo = fopen(nomeArquivo, "a");  // abre em modo append
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fprintf(arquivo, "%.2f,%.0f\n", tempo,valorConvergencia);
    fclose(arquivo);
}

int main() {
    int n = 20;
    char nomeArquivoCSV[] = "distancia_matrix.csv";
    int** matriz = ler_matriz_csv(nomeArquivoCSV, n);

    if (!matriz) {
        return 1;
    }

    double rhos[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95};
    int formigas[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // double rho = 0.5;
    // int numFormigas = 50;
    double alpha = 1.0;
    double beta = 2.0;
    int Q = 100;
    int maxIter = 500;

    //antColony(matriz, n, rho, numFormigas, alpha, beta, Q, maxIter);

    // Variando rho
    
    for (int i = 0; i < 10; i++) {
        double rho = rhos[i];
        char nomeArquivo[100];
        sprintf(nomeArquivo, "convergencia_rho_%.2f.csv", rho);

        for (int rep = 0; rep < 10; rep++) {
            double* resultados = antColony(matriz, n, rho, 50, alpha, beta, Q, maxIter);
            salvarTempoCSV(resultados[0], resultados[1], nomeArquivo);
            free(resultados);
        }
    }

    // Variando número de formigas
    for (int i = 0; i < 10; i++) {
        int numAnts = formigas[i];
        char nomeArquivo[100];
        sprintf(nomeArquivo, "convergencia_ants_%d.csv", numAnts);

        for (int rep = 0; rep < 10; rep++) {
            double* resultados = antColony(matriz, n, 0.5, numAnts, alpha, beta, Q, maxIter);
            salvarTempoCSV(resultados[0], resultados[1], nomeArquivo);
            free(resultados);
        }
    } 

    // liberar matriz
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);

    return 0;
}