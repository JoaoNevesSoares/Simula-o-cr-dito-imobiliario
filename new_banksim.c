#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
int SALDO,SUB,TAXAS;

struct recursosCliente
{

    int entrada;
    int sub;
    int taxas;
    int total;
    int id;
};

int threadsExecuting, threadsWaiting;
pthread_cond_t condicional;
pthread_mutex_t mutex;


void *cliente(void *id_cliente){

    struct recursosCliente *cliente = malloc(sizeof(struct recursosCliente));
    cliente->id  = (int) id_cliente;
    pthread_mutex_lock(&mutex);
    cliente->total = (rand() % SALDO);
    pthread_mutex_unlock(&mutex);
    cliente->entrada = (rand() % (int)(cliente->total * 0.7 - cliente->total * 0.4 + 1)) + cliente->total * 0.4;
    cliente->taxas = (rand() % (int)(cliente->total * 0.05 - (cliente->total * 0.05) * 0.5 + 1)) + (cliente->total * 0.05) * 0.5;
    switch (rand() % 4)
            {
            case 0: //No código do luanzin nao tinha essa possibilidade
                cliente->sub = 0;
                break;    
            case 1:
                cliente->sub = ceil(cliente->total * 0.05);
                break;
            case 2:
                cliente->sub = ceil(cliente->total * 0.10);
                break;
            case 3:
                cliente->sub = ceil(cliente->total * 0.20);
                break;

            default:
                break;
            }
    pthread_mutex_lock(&mutex);
    while(1){

        printf("Cliente %d na região crítica\n\n", cliente->id);
        if(SALDO >= (cliente->total - (cliente->entrada +cliente->sub)) && SUB >= cliente->sub && TAXAS >= cliente->taxas){

            printf("Empréstimo aceito para cliente %d\n"
                   "Total imóvel: %d\n"
                   "Entrada: %d (%d%%) | Recursos banco: %d\n"
                   "Subsídio: %d (%d%%) | Subsídio banco: %d\n"
                   "Taxas: %d (%d%%) | Taxas banco %d\n"
                   "\n--------\n",
                   cliente->id,
                   cliente->total,
                   cliente->entrada, (int)(100 * cliente->entrada / cliente->total), SALDO,
                   cliente->sub, (int)(100 * cliente->sub / cliente->total), SUB,
                   cliente->taxas, (int)((cliente->taxas / (cliente->total * 0.05)) * 100), TAXAS);
            SALDO -= (cliente->total -(cliente->entrada + cliente->sub));
            SUB -= cliente->sub;
            TAXAS -= (cliente->total * 0.05 - cliente->taxas);
            
            pthread_mutex_unlock(&mutex);
            
            sleep((rand()%4)+1);

            pthread_mutex_lock(&mutex);
            SALDO +=cliente->entrada;
            SUB += cliente->sub; //corrigido da versao do luan
            TAXAS += cliente->taxas;
            printf("\nCliente %d retornou seu empréstimo\n\n", cliente->id);
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&condicional);
            threadsExecuting --; //é regiao crítica ?
            free(cliente);
            pthread_exit(NULL);
            break; // acho que nao precisa desse break pois pthread_exit retorna para a thread que chamou cliente
        }
        else{

            printf("Empréstimo recusado para cliente %d\n"
                   "Total imóvel: %d\n"
                   "Entrada: %d (%d%%) | Recursos banco: %d\n"
                   "Subsídio: %d (%d%%) | Subsídio banco: %d\n"
                   "Taxas: %d (%d%%) | Taxas banco %d\n"
                   "\n--------\n",
                   cliente->id,
                   cliente->total,
                   cliente->entrada, (int)(100 * cliente->entrada / cliente->total), SALDO,
                   cliente->sub, (int)(100 * cliente->sub / cliente->total), SUB,
                   cliente->taxas, (int)((cliente->taxas / (cliente->total * 0.05)) * 100), TAXAS);
            threadsWaiting++; // regiao crítica ?
            if(threadsWaiting == threadsExecuting){ //regiao crítica ??
                printf("\n\nAzedou, estado inseguro, finalizando programa...\n");
                exit(1);
            }
            pthread_cond_wait(&condicional,&mutex);
            threadsWaiting--; // não entendi            
        }
        
    }
    
}
int main(int argc,char **argv){

    long status;
    int qntThreads;
    int qntGroups;
    if(argc > 1){
        qntThreads = atoi(argv[1]);
        qntGroups = atoi(argv[2]);
        SALDO = atoi(argv[3]);
        SUB = atoi(argv[4]);
        TAXAS = atoi(argv[5]);
    }
    else{
        qntThreads = 15;
        qntGroups = 3;
        SALDO = 500000;
        SUB = 100000;
        TAXAS = 30000;
    }
    int sub_original = SUB;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condicional, NULL);
    srand(time(0));
    for(int j=0;j<qntGroups;j++){

        pthread_t threads[qntThreads];
        threadsWaiting =0;
        threadsExecuting = qntThreads;
        for(long i=0;i<qntThreads;i++){

            status  = pthread_create(&threads[i],NULL,cliente,(void *)i);
            if(status){

                printf("pthread_create fail\n");
                exit(1);
            }
        }
        for(int i=0;i<qntThreads;i++){
            status = pthread_join(threads[i],NULL);
            if(status){
                printf("pthread_join fail\n");
                exit(1);
            }
        }
        SUB += sub_original*0.5; // bônus por atender um grupo de clientes
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condicional);
    pthread_exit(NULL);
}