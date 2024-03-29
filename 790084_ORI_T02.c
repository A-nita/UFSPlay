/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Organização de Recuperação da Informação
 * Prof. Tiago A. Almeida
 *
 * Trabalho 02 - Árvores-B
 *
 * RA: 
 * Aluno: 
 * ========================================================================== */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

typedef enum {false, true} bool;

/* Tamanho dos campos dos registros */
/* Campos de tamanho fixo */
#define TAM_ID_USER 12
#define TAM_CELULAR 12
#define TAM_SALDO 14
#define TAM_DATE 9
#define TAM_ID_GAME 9
#define QTD_MAX_CATEGORIAS 3

/* Campos de tamanho variável (tamanho máximo) */
#define TAM_MAX_USER 48
#define TAM_MAX_TITULO 44
#define TAM_MAX_EMPRESA 48
#define TAM_MAX_EMAIL 42
#define TAM_MAX_CATEGORIA 20

#define MAX_REGISTROS 1000
#define TAM_REGISTRO_USUARIO (TAM_ID_USER+TAM_MAX_USER+TAM_MAX_EMAIL+TAM_SALDO+TAM_CELULAR)
#define TAM_REGISTRO_JOGO 256
#define TAM_REGISTRO_COMPRA (TAM_ID_USER+TAM_DATE+TAM_ID_GAME-3)
#define TAM_ARQUIVO_USUARIO (TAM_REGISTRO_USUARIO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGO (TAM_REGISTRO_JOGO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRA (TAM_REGISTRO_COMPRA * MAX_REGISTROS + 1)

#define TAM_RRN_REGISTRO 4
#define TAM_CHAVE_USUARIOS_IDX (TAM_ID_USER + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_JOGOS_IDX (TAM_ID_GAME + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_COMPRAS_IDX (TAM_ID_USER + TAM_ID_GAME + TAM_RRN_REGISTRO - 2)
#define TAM_CHAVE_TITULO_IDX (TAM_MAX_TITULO + TAM_ID_GAME - 2)
#define TAM_CHAVE_DATA_USER_GAME_IDX (TAM_DATE + TAM_ID_USER + TAM_ID_GAME - 3)
#define TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX (TAM_MAX_CATEGORIA - 1)
#define TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX (TAM_ID_GAME - 1)

#define TAM_ARQUIVO_USUARIOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRAS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_TITULO_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_DATA_USER_GAME_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CATEGORIAS_IDX (1000 * MAX_REGISTROS + 1)

/* Mensagens padrões */
#define SUCESSO                          "OK\n"
#define RRN_NOS                          "Nos percorridos:"
#define RRN_REGS_PRIMARIOS               "Registros primários percorridos:"
#define RRN_REGS_SECUNDARIOS             "Registros secundários percorridos:"
#define AVISO_NENHUM_REGISTRO_ENCONTRADO "AVISO: Nenhum registro encontrado\n"
#define ERRO_OPCAO_INVALIDA              "ERRO: Opcao invalida\n"
#define ERRO_MEMORIA_INSUFICIENTE        "ERRO: Memoria insuficiente\n"
#define ERRO_PK_REPETIDA                 "ERRO: Ja existe um registro com a chave %s\n"
#define ERRO_REGISTRO_NAO_ENCONTRADO     "ERRO: Registro nao encontrado\n"
#define ERRO_SALDO_NAO_SUFICIENTE        "ERRO: Saldo insuficiente\n"
#define ERRO_CATEGORIA_REPETIDA          "ERRO: O jogo %s ja possui a categoria %s\n"
#define ERRO_VALOR_INVALIDO              "ERRO: Valor invalido\n"
#define ERRO_ARQUIVO_VAZIO               "ERRO: Arquivo vazio\n"
#define ERRO_NAO_IMPLEMENTADO            "ERRO: Funcao %s nao implementada\n"

/* Registro de Usuario */
typedef struct {
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    double saldo;
} Usuario;

/* Registro de Jogo */
typedef struct {
    char id_game[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char data_lancamento[TAM_DATE];
    double preco;
    char categorias[QTD_MAX_CATEGORIAS][TAM_MAX_CATEGORIA];
} Jogo;

/* Registro de Compra */
typedef struct {
    char id_user_dono[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    char data_compra[TAM_DATE];
} Compra;


/*----- Registros dos índices -----*/
/* Struct para índice de lista invertida */
typedef struct {
    char* chave;
    int proximo_indice;
} inverted_list_node;

/* Struct para um nó de Árvore-B */
typedef struct {
    int this_rrn;
    int qtd_chaves;
    char **chaves; // ponteiro para o começo do campo de chaves no arquivo de índice respectivo
    bool folha;
    int *filhos; // vetor de int para o RRN dos nós filhos (DEVE SER DESALOCADO APÓS O USO!!!)
} btree_node;

/* Variáveis globais */
/* Arquivos de dados */
char ARQUIVO_USUARIOS[TAM_ARQUIVO_USUARIO];
char ARQUIVO_JOGOS[TAM_ARQUIVO_JOGO];
char ARQUIVO_COMPRAS[TAM_ARQUIVO_COMPRA];

/* Ordem das Árvores-B */
int btree_order = 3; // valor padrão

/* Índices */
/* Struct para os parâmetros de uma lista invertida */
typedef struct {
    // Ponteiro para o arquivo de índice secundário
    char *arquivo_secundario;

    // Ponteiro para o arquivo de índice primário
    char *arquivo_primario;

    // Quantidade de registros de índice secundário
    unsigned qtd_registros_secundario;

    // Quantidade de registros de índice primário
    unsigned qtd_registros_primario;

    // Tamanho de uma chave secundária nesse índice
    unsigned tam_chave_secundaria;

    // Tamanho de uma chave primária nesse índice
    unsigned tam_chave_primaria;

    // Função utilizada para comparar as chaves do índice secundário.
    // Igual às funções de comparação do bsearch e qsort.
    int (*compar)(const void *key, const void *elem);
} inverted_list;

/* Struct para os parâmetros de uma Árvore-B */
typedef struct {
    // RRN da raíz
    int rrn_raiz;

    // Ponteiro para o arquivo de índice
    char *arquivo;

    // Quantidade de nós no arquivo de índice
    unsigned qtd_nos;

    // Tamanho de uma chave nesse índice
    unsigned tam_chave;

    // Função utilizada para comparar as chaves do índice.
    // Igual às funções de comparação do bsearch e qsort.
    int (*compar)(const void *key, const void *elem);
} btree;

typedef struct {
    char chave_promovida[TAM_CHAVE_TITULO_IDX + 1]; // TAM_CHAVE_TITULO_IDX é a maior chave possível
    int filho_direito;
} promovido_aux;

/* Arquivos de índices */
char ARQUIVO_USUARIOS_IDX[TAM_ARQUIVO_USUARIOS_IDX];
char ARQUIVO_JOGOS_IDX[TAM_ARQUIVO_JOGOS_IDX];
char ARQUIVO_COMPRAS_IDX[TAM_ARQUIVO_COMPRAS_IDX];
char ARQUIVO_TITULO_IDX[TAM_ARQUIVO_TITULO_IDX];
char ARQUIVO_DATA_USER_GAME_IDX[TAM_ARQUIVO_DATA_USER_GAME_IDX];
char ARQUIVO_CATEGORIAS_SECUNDARIO_IDX[TAM_ARQUIVO_CATEGORIAS_IDX];
char ARQUIVO_CATEGORIAS_PRIMARIO_IDX[TAM_ARQUIVO_CATEGORIAS_IDX];

/* Comparam a chave (key) com cada elemento do índice (elem).
 * Funções auxiliares para o buscar e inserir chaves em Árvores-B.
 * Note que, desta vez, as funções comparam chaves no formato de strings, não structs.
 * key é a chave do tipo string que está sendo buscada ou inserida. elem é uma chave do tipo string da struct btree_node.
 *
 * Dica: pesquise sobre as funções strncmp e strnlen. Muito provavelmente vai querer utilizá-las no código!
 * */
int order_usuarios_idx(const void *key, const void *elem);
int order_jogos_idx(const void *key, const void *elem);
int order_compras_idx(const void *key, const void *elem);
int order_titulo_idx(const void *key, const void *elem);
int order_data_user_game_idx(const void *key, const void *elem);
int order_categorias_idx(const void *key, const void *elem);

btree usuarios_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_USUARIOS_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_USUARIOS_IDX,
    .compar = order_usuarios_idx,
};

btree jogos_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_JOGOS_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_JOGOS_IDX,
    .compar = order_jogos_idx,
};

btree compras_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_COMPRAS_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_COMPRAS_IDX,
    .compar = order_compras_idx,
};

btree titulo_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_TITULO_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_TITULO_IDX,
    .compar = order_titulo_idx,
};

btree data_user_game_idx = {
    .rrn_raiz = -1,
    .arquivo = ARQUIVO_DATA_USER_GAME_IDX,
    .qtd_nos = 0,
    .tam_chave = TAM_CHAVE_DATA_USER_GAME_IDX,
    .compar = order_data_user_game_idx,
};

inverted_list categorias_idx = {
    .arquivo_secundario = ARQUIVO_CATEGORIAS_SECUNDARIO_IDX,
    .arquivo_primario = ARQUIVO_CATEGORIAS_PRIMARIO_IDX,
    .qtd_registros_secundario = 0,
    .qtd_registros_primario = 0,
    .tam_chave_secundaria = TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX,
    .tam_chave_primaria = TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX,
    .compar = order_categorias_idx,
};

/* Contadores */
unsigned qtd_registros_usuarios = 0;
unsigned qtd_registros_jogos = 0;
unsigned qtd_registros_compras = 0;

/* Funções de geração determinística de números pseudo-aleatórios */
uint64_t prng_seed;

void prng_srand(uint64_t value) {
    prng_seed = value;
}

uint64_t prng_rand() {
    // https://en.wikipedia.org/wiki/Xorshift#xorshift*
    uint64_t x = prng_seed; // O estado deve ser iniciado com um valor diferente de 0
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    prng_seed = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

/* Funções de manipulação de data */
int64_t epoch;

void set_time(int64_t value) {
    epoch = value;
}

void tick_time() {
    epoch += prng_rand() % 864000; // 10 dias
}

struct tm gmtime_(const int64_t lcltime) {
    // based on https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/time/gmtime_r.c;
    struct tm res;
    long days = lcltime / 86400 + 719468;
    long rem = lcltime % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }

    res.tm_hour = (int) (rem / 3600);
    rem %= 3600;
    res.tm_min = (int) (rem / 60);
    res.tm_sec = (int) (rem % 60);

    int weekday = (3 + days) % 7;
    if (weekday < 0) weekday += 7;
    res.tm_wday = weekday;

    int era = (days >= 0 ? days : days - 146096) / 146097;
    unsigned long eraday = days - era * 146097;
    unsigned erayear = (eraday - eraday / 1460 + eraday / 36524 - eraday / 146096) / 365;
    unsigned yearday = eraday - (365 * erayear + erayear / 4 - erayear / 100);
    unsigned month = (5 * yearday + 2) / 153;
    unsigned day = yearday - (153 * month + 2) / 5 + 1;
    month += month < 10 ? 2 : -10;

    int isleap = ((erayear % 4) == 0 && (erayear % 100) != 0) || (erayear % 400) == 0;
    res.tm_yday = yearday >= 306 ? yearday - 306 : yearday + 59 + isleap;
    res.tm_year = (erayear + era * 400 + (month <= 1)) - 1900;
    res.tm_mon = month;
    res.tm_mday = day;
    res.tm_isdst = 0;

    return res;
}

/**
 * Escreve a <i>data</i> atual no formato <code>AAAAMMDD</code> em uma <i>string</i>
 * fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char timestamp[TAM_DATE];<br />
 * current_date(date);<br />
 * printf("data atual: %s&#92;n", date);<br />
 * </code>
 *
 * @param buffer String de tamanho <code>TAM_DATE</code> no qual será escrita
 * a <i>timestamp</i>. É terminado pelo caractere <code>\0</code>.
 */
void current_date(char buffer[TAM_DATE]) {
    // http://www.cplusplus.com/reference/ctime/strftime/
    // http://www.cplusplus.com/reference/ctime/gmtime/
    // AAAA MM DD
    // %Y   %m %d
    struct tm tm_ = gmtime_(epoch);
    strftime(buffer, TAM_DATE, "%Y%m%d", &tm_);
}

/* Remove comentários (--) e caracteres whitespace do começo e fim de uma string */
void clear_input(char *str) {
    char *ptr = str;
    int len = 0;

    for (; ptr[len]; ++len) {
        if (strncmp(&ptr[len], "--", 2) == 0) {
            ptr[len] = '\0';
            break;
        }
    }

    while(len-1 > 0 && isspace(ptr[len-1]))
        ptr[--len] = '\0';

    while(*ptr && isspace(*ptr))
        ++ptr, --len;

    memmove(str, ptr, len + 1);
}


/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Cria o índice respectivo */
void criar_usuarios_idx();
void criar_jogos_idx();
void criar_compras_idx();
void criar_titulo_idx();
void criar_data_user_game_idx();
void criar_categorias_idx();

/* Exibe um registro com base no RRN */
bool exibir_usuario(int rrn);
bool exibir_jogo(int rrn);
bool exibir_compra(int rrn);

/* Exibe um registro com base na chave de um btree_node */
bool exibir_btree_usuario(char *chave);
bool exibir_btree_jogo(char *chave);
bool exibir_btree_compra(char *chave);
bool exibir_btree_titulo(char *chave);
bool exibir_btree_data_user_game(char *chave);

/* Recupera do arquivo o registro com o RRN informado
 * e retorna os dados nas struct Cliente/Transacao */
Usuario recuperar_registro_usuario(int rrn);
Jogo recuperar_registro_jogo(int rrn);
Compra recuperar_registro_compra(int rrn);

/* Escreve no arquivo respectivo na posição informada (RRN)
 * os dados na struct Cliente/Transacao */
void escrever_registro_usuario(Usuario u, int rrn);
void escrever_registro_jogo(Jogo j, int rrn);
void escrever_registro_compra(Compra c, int rrn);

/* Funções principais */
void cadastrar_usuario_menu(char* id_user, char* username, char* email);
void cadastrar_celular_menu(char* id_user, char* celular);
void cadastrar_jogo_menu(char* titulo, char* desenvolvedor, char* editora, char* lancamento, double preco);
void adicionar_saldo_menu(char* id_user, double valor);
void comprar_menu(char* id_user, char* titulo);
void cadastrar_categoria_menu(char* titulo, char* categoria);

/* Busca */
void buscar_usuario_id_user_menu(char *id_user);
void buscar_jogo_id_menu(char *id_game);
void buscar_jogo_titulo_menu(char *titulo);

/* Listagem */
void listar_usuarios_id_user_menu();
void listar_jogos_categorias_menu(char *categoria);
void listar_compras_periodo_menu(char *data_inicio, char *data_fim);

/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu();
void imprimir_arquivo_jogos_menu();
void imprimir_arquivo_compras_menu();

/* Imprimir arquivos de índice */
void imprimir_usuarios_idx_menu();
void imprimir_jogos_idx_menu();
void imprimir_compras_idx_menu();
void imprimir_titulo_idx_menu();
void imprimir_data_user_game_idx_menu();
void imprimir_categorias_secundario_idx_menu();
void imprimir_categorias_primario_idx_menu();


/* Funções de manipulação de Lista Invertida */
/**
 * Responsável por inserir duas chaves (chave_secundaria e chave_primaria) em uma Lista Invertida (t).<br />
 * Atualiza os parâmetros dos índices primário e secundário conforme necessário.<br />
 * As chaves a serem inseridas devem estar no formato correto e com tamanho t->tam_chave_primario e t->tam_chave_secundario.<br />
 * O funcionamento deve ser genérico para qualquer Lista Invertida, adaptando-se para os diferentes parâmetros presentes em seus structs.<br />
 *
 * @param chave_secundaria Chave a ser buscada (caso exista) ou inserida (caso não exista) no registro secundário da Lista Invertida.
 * @param chave_primaria Chave a ser inserida no registro primário da Lista Invertida.
 * @param t Ponteiro para a Lista Invertida na qual serão inseridas as chaves.
 */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t);

/**
 * Responsável por buscar uma chave no índice secundário de uma Lista invertida (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice inicial das chaves no registro primário.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = inverted_list_secondary_search(NULL, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse no caminho feito para encontrar a chave.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, true, categoria, &categorias_idx);<br />
 * </code>
 *
 * @param result Ponteiro para ser escrito o índice inicial (primeira ocorrência) das chaves do registro primário. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Árvore-B.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_secondary_search(int* result, bool exibir_caminho, char *chave, inverted_list *t);

/**
 * Responsável por percorrer o índice primário de uma Lista invertida (T). O valor de retorno indica a quantidade de chaves encontradas.
 * O ponteiro para o vetor de strings result pode ser fornecido opcionalmente, e será populado com a lista de todas as chaves encontradas.
 * O ponteiro para o inteiro indice_final também pode ser fornecido opcionalmente, e deve conter o índice do último campo da lista encadeada 
 * da chave primaria fornecida (isso é útil na inserção de um novo registro).<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. As chaves encontradas deverão ser retornadas e tanto o caminho quanto o indice_final não devem ser informados.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, false, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse nas chaves encontradas, apenas no indice_final, e o caminho não deve ser informado.<br />
 * ...<br />
 * int indice_final;
 * int qtd = inverted_list_primary_search(NULL, false, indice, &indice_final, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse nas chaves encontradas e no caminho feito.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, true, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * </code>
 *
 * @param indice_final Ponteiro para ser escrito o índice do último registro encontrado. É ignorado caso NULL.
 * @param result Ponteiro para serem escritas as chaves encontradas. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param indice Índice do primeiro registro da lista encadeada a ser procurado.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica a quantidade de chaves encontradas.
 */
int inverted_list_primary_search(int* indice_final, char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, inverted_list *t);

/**
 * Responsável por buscar uma chave (k) dentre os registros secundários de uma Lista Invertida de forma eficiente.<br />
 * O valor de retorno deve indicar se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice no registro secundário da chave encontrada.<br />
 *
 * @param result Ponteiro para ser escrito o índice nos registros secundários da chave encontrada. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Lista Invertida.
 * @param t Ponteiro para o índice da Lista Invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_binary_search(int* result, bool exibir_caminho, char *chave, inverted_list *t);

/* Funções de manipulação de Árvores-B */
/**
 * Responsável por inserir uma chave (k) em uma Árvore-B (T). Atualiza os parâmetros da Árvore-B conforme necessário.<br />
 * A chave a ser inserida deve estar no formato correto e com tamanho t->tam_chave.<br />
 * O funcionamento deve ser genérico para qualquer Árvore-B, considerando que os únicos parâmetros que se alteram entre
 * as árvores é o t->tam_chave.<br />
 * Comportamento de acordo com as especificações do PDF sobre Árvores-B e suas operações.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * ...<br />
 * char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];<br />
 * sprintf(usuario_str, "%s%04d", id_user, rrn_usuario);<br />
 * btree_insert(usuario_str, &usuarios_idx);<br />
 * ...<br />
 * </code>
 *
 * @param chave Chave a ser inserida na Árvore-B.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 */
void btree_insert(char *chave, btree *t);

/**
 * Função auxiliar de inserção de uma chave (k) em uma Árvore-B (T). Atualiza os parâmetros da Árvore-B conforme necessário.<br />
 * Esta é uma função recursiva. Ela recebe o RRN do nó a ser trabalhado sobre.<br />
 * Comportamento de acordo com as especificações do PDF sobre Árvores-B e suas operações.<br />
 *
 * @param chave Chave a ser inserida na Árvore-B.
 * @param rrn RRN do nó no qual deverá ser processado.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 * @return Retorna uma struct do tipo promovido_aux que contém a chave promovida e o RRN do filho direito.
 */
promovido_aux btree_insert_aux(char *chave, int rrn, btree *t);

/**
 * Função auxiliar para dividir um nó de uma Árvore-B (T). Atualiza os parâmetros conforme necessário.<br />
 * Comportamento de acordo com as especificações do PDF sobre Árvores-B e suas operações.<br />
 *
 * @param chave Chave a ser inserida na Árvore-B.
 * @param filho_direito RRN do filho direito.
 * @param rrn RRN do nó no qual deverá ser processado.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 * @return Retorna uma struct do tipo promovido_aux que contém a chave promovida e o RRN do filho direito.
 */
promovido_aux btree_divide(char *chave, int filho_direito, int rrn, btree *t);

/**
 * Responsável por buscar uma chave (k) em uma Árvore-B (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para a string result pode ser fornecido opcionalmente, e conterá o resultado encontrado.<br />
 * Esta é uma função recursiva. O parâmetro rrn recebe a raíz da Árvore-B na primeira chamada, e nas chamadas
 * subsequentes é o RRN do filho de acordo com o algoritmo fornecido.<br />
 * Comportamento de acordo com as especificações do PDF sobre Árvores-B e suas operações.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * char result[TAM_CHAVE_CLIENTES_IDX + 1];<br />
 * bool found = btree_search(result, false, id_user, clientes_idx.rrn_raiz, &clientes_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = btree_search(NULL, false, id_user, clientes_idx.rrn_raiz, &clientes_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Busca por uma chave de tamanho variável (específico para o caso de buscas por chaves PIX).<br />
 * ...<br />
 * char chave_pix_str[TAM_MAX_CHAVE_PIX];<br />
 * strcpy(chave_pix_str, c.chaves_pix[i] + 1);<br />
 * strpadright(chave_pix_str, '#', TAM_MAX_CHAVE_PIX - 1);<br />
 * bool found = btree_search(NULL, false, chave_pix_str, chaves_pix_idx.rrn_raiz, &chaves_pix_idx);<br />
 * ...<br />
 * <br />
* // Exemplo 4. Há interesse no caminho feito para encontrar a chave.<br />
  * ...<br />
 * char result[TAM_CHAVE_CLIENTES_IDX + 1];<br />
 * printf(RRN_NOS);<br />
 * bool found = btree_search(result, true, id_user, clientes_idx.rrn_raiz, &clientes_idx);<br />
 * printf("\n");<br />
 * </code>
 *
 * @param result Ponteiro para ser escrita a chave encontrada. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Árvore-B.
 * @param rrn RRN do nó no qual deverá ser processado. É o RRN da raíz da Árvore-B caso seja a primeira chamada.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool btree_search(char *result, bool exibir_caminho, char *chave, int rrn, btree *t);

/**
 * Responsável por buscar uma chave (k) dentro do nó de uma Árvore-B (T) de forma eficiente. O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e indica o índice da chave encontrada (caso tenha sido encontrada) 
 * ou o índice do filho onde esta chave deve estar (caso não tenha sido encontrada).<br />
 *
 * @param result Ponteiro para ser escrita a chave encontrada. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave Chave a ser buscada na Árvore-B.
 * @param node Ponteiro para o nó onde a busca deve ser feita.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool btree_binary_search(int *result, bool exibir_caminho, char* chave, btree_node* node, btree* t);

/**
 * Função para percorrer uma Árvore-B (T) em ordem.<br />
 * Os parâmetros chave_inicio e chave_fim podem ser fornecidos opcionalmente, e contém o intervalo do percurso.
 * Caso chave_inicio e chave_fim sejam NULL, o índice inteiro é percorrido.
 * Esta é uma função recursiva. O parâmetro rrn recebe a raíz da Árvore-B na primeira chamada, e nas chamadas
 * subsequentes é o RRN do filho de acordo com o algoritmo de percursão em ordem.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * // Exemplo 1. Intervalo não especificado.
 * ...<br />
 * bool imprimiu = btree_print_in_order(NULL, NULL, exibir_btree_cliente, clientes_idx.rrn_raiz, &clientes_idx);
 * ...<br />
 * <br />
 * // Exemplo 2. Imprime as transações contidas no intervalo especificado.
 * ...<br />
 * bool imprimiu = btree_print_in_order(data_inicio, data_fim, exibir_btree_timestamp_id_user_origem, timestamp_id_user_origem_idx.rrn_raiz, &timestamp_id_user_origem_idx);
 * ...<br />
 * </code>
 *
 * @param chave_inicio Começo do intervalo. É ignorado caso NULL.
 * @param chave_fim Fim do intervalo. É ignorado caso NULL.
 * @param exibir Função utilizada para imprimir uma chave no índice. É informada a chave para a função.
 * @param rrn RRN do nó no qual deverá ser processado.
 * @param t Ponteiro para o índice do tipo Árvore-B no qual será inserida a chave.
 * @return Indica se alguma chave foi impressa.
 */
bool btree_print_in_order(char *chave_inicio, char *chave_fim, bool (*exibir)(char *chave), int rrn, btree *t);

/**
 * Função interna para ler um nó em uma Árvore-B (T).<br />
 *
 * @param no No a ser lido da Árvore-B.
 * @param t Árvore-B na qual será feita a leitura do nó.
 */
btree_node btree_read(int rrn, btree *t);

/**
 * Função interna para escrever um nó em uma Árvore-B (T).<br />
 *
 * @param no No a ser escrito na Árvore-B.
 * @param t Árvore-B na qual será feita a escrita do nó.
 */
void btree_write(btree_node no, btree *t);

/**
 * Função interna para alocar o espaço necessário dos campos chaves (vetor de strings) e filhos (vetor de inteiros) da struct btree_node.<br />
 *
 * @param t Árvore-B base para o qual será alocado um struct btree_node.
 */
btree_node btree_node_malloc(btree *t);

/**
 * Função interna para liberar o espaço alocado dos campos chaves (vetor de strings) e filhos (vetor de inteiros) da struct btree_node.<br />
 *
 * @param t Árvore-B base para o qual será liberado o espaço alocado para um struct btree_node.
 */
void btree_node_free(btree_node no);

/**
 * Função interna para calcular o tamanho em bytes de uma Árvore-B.<br />
 *
 * @param t Árvore-B base para o qual será calculado o tamanho.
 */
int btree_register_size(btree *t);

/**
 * Preenche uma string str com o caractere pad para completar o tamanho size.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 * @param pad Caractere utilizado para fazer o preenchimento à direita.
 * @param size Tamanho desejado para a string.
 */
char* strpadright(char *str, char pad, unsigned size);

/* <<< COLOQUE AQUI OS DEMAIS PROTÓTIPOS DE FUNÇÕES, SE NECESSÁRIO >>> */
int qsort_string_categoria(const void *a, const void *b) {
    return strcmp((char *) a, (char *) b);
}

void add_usuario_idx();
void add_id_jogo_idx();
void add_compra_idx();
void add_titulo_idx();
void add_data_user_game_idx();

/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */

int main() {
    // variáveis utilizadas pelo interpretador de comandos
    char input[500];
    uint64_t seed = 2;
    uint64_t time = 1616077800; // UTC 18/03/2021 14:30:00
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    char id[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char lancamento[TAM_DATE];
    char categoria[TAM_MAX_CATEGORIA];
    double valor;
    char data_inicio[TAM_DATE];
    char data_fim[TAM_DATE];

    scanf("SET BTREE_ORDER %d;\n", &btree_order);

    scanf("SET ARQUIVO_USUARIOS '%[^\n]\n", ARQUIVO_USUARIOS);
    int temp_len = strlen(ARQUIVO_USUARIOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_usuarios = (temp_len - 2) / TAM_REGISTRO_USUARIO;
    ARQUIVO_USUARIOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_JOGOS '%[^\n]\n", ARQUIVO_JOGOS);
    temp_len = strlen(ARQUIVO_JOGOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_jogos = (temp_len - 2) / TAM_REGISTRO_JOGO;
    ARQUIVO_JOGOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_COMPRAS '%[^\n]\n", ARQUIVO_COMPRAS);
    temp_len = strlen(ARQUIVO_COMPRAS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_compras = (temp_len - 2) / TAM_REGISTRO_COMPRA;
    ARQUIVO_COMPRAS[temp_len - 2] = '\0';

    // inicialização do gerador de números aleatórios e função de datas
    prng_srand(seed);
    set_time(time);
    //todo
    criar_usuarios_idx();
    criar_jogos_idx();
    criar_compras_idx();
    criar_titulo_idx();
    criar_data_user_game_idx();
//    criar_categorias_idx();

    while (1) {
        fgets(input, 500, stdin);
        printf("%s", input);
        clear_input(input);

        if (strcmp("", input) == 0)
            continue; // não avança o tempo nem imprime o comando este seja em branco

        /* Funções principais */
        if (sscanf(input, "INSERT INTO usuarios VALUES ('%[^']', '%[^']', '%[^']');", id_user, username, email) == 3)
            cadastrar_usuario_menu(id_user, username, email);
        else if (sscanf(input, "UPDATE usuarios SET celular = '%[^']' WHERE id_user = '%[^']';", celular, id_user) == 2)
            cadastrar_celular_menu(id_user, celular);
        else if (sscanf(input, "INSERT INTO jogos VALUES ('%[^']', '%[^']', '%[^']', '%[^']', %lf);", titulo, desenvolvedor, editora, lancamento, &valor) == 5)
            cadastrar_jogo_menu(titulo, desenvolvedor, editora, lancamento, valor);
        else if (sscanf(input, "UPDATE usuarios SET saldo = saldo + %lf WHERE id_user = '%[^']';", &valor, id_user) == 2)
            adicionar_saldo_menu(id_user, valor);
        else if (sscanf(input, "INSERT INTO compras VALUES ('%[^']', '%[^']');", id_user, titulo) == 2)
            comprar_menu(id_user, titulo);
        else if (sscanf(input, "UPDATE jogos SET categorias = array_append(categorias, '%[^']') WHERE titulo = '%[^']';", categoria, titulo) == 2)
            cadastrar_categoria_menu(titulo, categoria);

        /* Busca */
        else if (sscanf(input, "SELECT * FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            buscar_usuario_id_user_menu(id_user);
        else if (sscanf(input, "SELECT * FROM jogos WHERE id_game = '%[^']';", id) == 1)
            buscar_jogo_id_menu(id);
        else if (sscanf(input, "SELECT * FROM jogos WHERE titulo = '%[^']';", titulo) == 1)
            buscar_jogo_titulo_menu(titulo);

        /* Listagem */
        else if (strcmp("SELECT * FROM usuarios ORDER BY id_user ASC;", input) == 0)
            listar_usuarios_id_user_menu();
        else if (sscanf(input, "SELECT * FROM jogos WHERE '%[^']' = ANY (categorias) ORDER BY id_game ASC;", categoria) == 1)
            listar_jogos_categorias_menu(categoria);
        else if (sscanf(input, "SELECT * FROM compras WHERE data_compra BETWEEN '%[^']' AND '%[^']' ORDER BY data_compra ASC;", data_inicio, data_fim) == 2)
            listar_compras_periodo_menu(data_inicio, data_fim);

        /* Imprimir arquivos de dados */
        else if (strcmp("\\echo file ARQUIVO_USUARIOS", input) == 0)
            imprimir_arquivo_usuarios_menu();
        else if (strcmp("\\echo file ARQUIVO_JOGOS", input) == 0)
            imprimir_arquivo_jogos_menu();
        else if (strcmp("\\echo file ARQUIVO_COMPRAS", input) == 0)
            imprimir_arquivo_compras_menu();

        /* Imprimir arquivos de índice */
        else if (strcmp("\\echo index usuarios_idx", input) == 0)
            imprimir_usuarios_idx_menu();
        else if (strcmp("\\echo index jogos_idx", input) == 0)
            imprimir_jogos_idx_menu();
        else if (strcmp("\\echo index compras_idx", input) == 0)
            imprimir_compras_idx_menu();
        else if (strcmp("\\echo index titulo_idx", input) == 0)
            imprimir_titulo_idx_menu();
        else if (strcmp("\\echo index data_user_game_idx", input) == 0)
            imprimir_data_user_game_idx_menu();
        else if (strcmp("\\echo index categorias_secundario_idx", input) == 0)
            imprimir_categorias_secundario_idx_menu();
        else if (strcmp("\\echo index categorias_primario_idx", input) == 0)
            imprimir_categorias_primario_idx_menu();

        /* Encerrar o programa */
        else if (strcmp("\\q", input) == 0)
            { return 0; }
        else if (sscanf(input, "SET SRAND %lu;", &seed) == 1)
            { prng_srand(seed); printf(SUCESSO); continue; }
        else if (sscanf(input, "SET TIME %lu;", &time) == 1)
            { set_time(time); printf(SUCESSO); continue; }
        else
            printf(ERRO_OPCAO_INVALIDA);

        tick_time();
    }
}
//todo
/* Cria o índice primário usuarios_idx */
void criar_usuarios_idx() {
    char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];
    for (unsigned i = 0; i < qtd_registros_usuarios; ++i) {
        Usuario u = recuperar_registro_usuario(i);

        sprintf(usuario_str, "%s%04d", u.id_user, i);
        btree_insert(usuario_str, &usuarios_idx);
    }
}

/* Cria o índice primário jogos_idx */
void criar_jogos_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char jogo_str[TAM_CHAVE_JOGOS_IDX + 1];
    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo j = recuperar_registro_jogo(i);

        sprintf(jogo_str, "%s%04d", j.id_game, i);
        btree_insert(jogo_str, &jogos_idx);
    }
}

/* Cria o índice primário compras_idx */
void criar_compras_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char compra_str[TAM_CHAVE_COMPRAS_IDX + 1];

    for (int i = 0; i < qtd_registros_compras; ++i) {
        Compra c = recuperar_registro_compra(i);
        sprintf(compra_str, "%s%s%04d",c.id_user_dono, c.id_game, i);
        btree_insert(compra_str, &compras_idx);
    }
}

/* Cria o índice secundário titulo_idx */
void criar_titulo_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char titulo_str[TAM_CHAVE_TITULO_IDX + 1];
    char titulo_jogo[TAM_MAX_TITULO-1];
    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo j = recuperar_registro_jogo(i);
        strcpy(titulo_jogo, j.titulo);
        strpadright(titulo_jogo, '#', TAM_MAX_TITULO-1);
        sprintf(titulo_str, "%s%s", titulo_jogo, j.id_game);
        btree_insert(titulo_str, &titulo_idx);
    }
}

/* Cria o índice secundário data_user_game_idx */
void criar_data_user_game_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char compra_str[TAM_CHAVE_DATA_USER_GAME_IDX + 1];
    for (int i = 0; i < qtd_registros_compras; ++i) {
        Compra c = recuperar_registro_compra(i);

        sprintf(compra_str, "%s%s%s", c.data_compra, c.id_user_dono, c.id_game);
        btree_insert(compra_str, &data_user_game_idx);
    }


}

/* Cria os índices (secundário e primário) de categorias_idx */
void criar_categorias_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "criar_categorias_idx");
}


/* Exibe um usuario dado seu RRN */
bool exibir_usuario(int rrn) {
    if (rrn < 0)
        return false;

    Usuario u = recuperar_registro_usuario(rrn);

    printf("%s, %s, %s, %s, %.2lf\n", u.id_user, u.username, u.email, u.celular, u.saldo);
    return true;
}

/* Exibe um jogo dado seu RRN */
bool exibir_jogo(int rrn) {
    if (rrn < 0)
        return false;

    Jogo j = recuperar_registro_jogo(rrn);

    printf("%s, %s, %s, %s, %s, %.2lf\n", j.id_game, j.titulo, j.desenvolvedor, j.editora, j.data_lancamento, j.preco);
    return true;
}

/* Exibe uma compra dado seu RRN */
bool exibir_compra(int rrn) {
    if (rrn < 0)
        return false;

    Compra c = recuperar_registro_compra(rrn);

    printf("%s, %s, %s\n", c.id_user_dono, c.data_compra, c.id_game);

    return true;
}

bool exibir_btree_usuario(char *chave) {
    int rrn = atoi(chave + TAM_ID_USER);
    return exibir_usuario(rrn);
}

bool exibir_btree_jogo(char *chave) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    int rrn = atoi(chave + TAM_ID_GAME);
    return exibir_jogo(rrn);
}

bool exibir_btree_compra(char *chave) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    int rrn = atoi(chave+TAM_CHAVE_COMPRAS_IDX-1);
    return exibir_compra(rrn);
}

bool exibir_btree_titulo(char *chave) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return exibir_btree_jogo(chave + TAM_MAX_TITULO);
}

bool exibir_btree_data_user_game(char *chave) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char data[TAM_CHAVE_COMPRAS_IDX+1];
//    data = chave+TAM_DATE;
    strcpy(data, chave+TAM_DATE-1);
    char result[TAM_CHAVE_COMPRAS_IDX+1];
    if(!btree_search(result, false, data, compras_idx.rrn_raiz, &compras_idx)){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
    }
    return exibir_btree_compra(result);

}

/* Recupera do arquivo de usuários o registro com o RRN
 * informado e retorna os dados na struct Usuario */
Usuario recuperar_registro_usuario(int rrn) {
    Usuario u;
    char temp[TAM_REGISTRO_USUARIO + 1], *p;
    strncpy(temp, ARQUIVO_USUARIOS + (rrn * TAM_REGISTRO_USUARIO), TAM_REGISTRO_USUARIO);
    temp[TAM_REGISTRO_USUARIO] = '\0';

    p = strtok(temp, ";");
    strcpy(u.id_user, p);
    p = strtok(NULL, ";");
    strcpy(u.username, p);
    p = strtok(NULL, ";");
    strcpy(u.email, p);
    p = strtok(NULL, ";");
    strcpy(u.celular, p);
    p = strtok(NULL, ";");
    u.saldo = atof(p);
    p = strtok(NULL, ";");

    return u;
}

/* Recupera do arquivo de jogos o registro com o RRN
 * informado e retorna os dados na struct Jogo */
Jogo recuperar_registro_jogo(int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    Jogo j;
    //iniciamos as categorias como vazias
    j.categorias[0][0] = '\0';
    j.categorias[1][0] = '\0';
    j.categorias[2][0] = '\0';

    char temp[TAM_REGISTRO_JOGO + 1], *p, *categorias;
    categorias = "";
    strncpy(temp, ARQUIVO_JOGOS + (rrn * TAM_REGISTRO_JOGO), TAM_REGISTRO_JOGO);
    temp[TAM_REGISTRO_JOGO] = '\0';

    p = strtok(temp, ";");
    strcpy(j.id_game, p);
    p = strtok(NULL, ";");
    strcpy(j.titulo, p);
    p = strtok(NULL, ";");
    strcpy(j.desenvolvedor, p);
    p = strtok(NULL, ";");
    strcpy(j.editora, p);
    p = strtok(NULL, ";");
    strcpy(j.data_lancamento, p);
    p = strtok(NULL, ";");
    j.preco = atof(p);
    p = strtok(NULL, "|;#");

    int i = 0;
    //adcionamos as categoria, caso existam ao Jogo j
    while(p) {
        strcpy(j.categorias[i], p);
        p = strtok(NULL, "|;#");
        i++;
    }

    return j;
}

/* Recupera do arquivo de compras o registro com o RRN
 * informado e retorna os dados na struct Compra */
Compra recuperar_registro_compra(int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    Compra c;
    char temp[TAM_REGISTRO_COMPRA + 1];
    //copiamos a compra do arquivo para a String temporaria
    strncpy(temp, ARQUIVO_COMPRAS + (rrn * TAM_REGISTRO_COMPRA), TAM_REGISTRO_COMPRA);
    temp[TAM_REGISTRO_COMPRA] = '\0';

    //copiamos os dados da compra na Atring para a struct compra e a retornamos
    strncpy(c.id_user_dono, temp,12);
    c.id_user_dono[11] = '\0';
    strncpy(c.data_compra, temp+11,8);
    c.data_compra[8] = '\0';
    strncpy(c.id_game, temp+12+7,8);
    c.id_game[8] = '\0';

    return c;
}

/* Escreve no arquivo de usuários na posição informada (RRN)
 * os dados na struct Usuario */
void escrever_registro_usuario(Usuario u, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char temp[TAM_REGISTRO_USUARIO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, u.id_user);
    strcat(temp, ";");
    strcat(temp, u.username);
    strcat(temp, ";");
    strcat(temp, u.email);
    strcat(temp, ";");
    strcat(temp, u.celular);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", u.saldo);
    strcat(temp, p);
    strcat(temp, ";");

    for (int i = strlen(temp); i < TAM_REGISTRO_USUARIO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_USUARIOS + rrn*TAM_REGISTRO_USUARIO, temp, TAM_REGISTRO_USUARIO);
    ARQUIVO_USUARIOS[qtd_registros_usuarios*TAM_REGISTRO_USUARIO] = '\0';
}

/* Escreve no arquivo de jogos na posição informada (RRN)
 * os dados na struct Jogo */
void escrever_registro_jogo(Jogo j, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char temp[TAM_REGISTRO_JOGO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';
    strcpy(temp, j.id_game);
    strcat(temp, ";");
    strcat(temp, j.titulo);
    strcat(temp, ";");
    strcat(temp, j.desenvolvedor);
    strcat(temp, ";");
    strcat(temp, j.editora);
    strcat(temp, ";");
    strcat(temp, j.data_lancamento);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", j.preco);
    strcat(temp, p);
    strcat(temp, ";");

    //verificamos se há categorias
    for (int i = 0; i < 3; ++i) {
        if(strlen(j.categorias[i]) != 0) {
            // Concatena um pipe antes de escrever a próxima categoria
            if(i != 0) {
                strcat(temp, "|");
            }
            strcat(temp, j.categorias[i]);
        }
    }
    strcat(temp, ";");

    //completamos o resgistor com ##
    strpadright(temp, '#', TAM_REGISTRO_JOGO);
    //escrevemos o jogo no arquivo
    strncpy(ARQUIVO_JOGOS + rrn*TAM_REGISTRO_JOGO, temp, TAM_REGISTRO_JOGO);
    ARQUIVO_JOGOS[qtd_registros_jogos*TAM_REGISTRO_JOGO] = '\0';
}

/* Escreve no arquivo de compras na posição informada (RRN)
 * os dados na struct Compra */
void escrever_registro_compra(Compra c, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    //    copiamos os dados da struct comprar para uma String temporaria
    char temp[TAM_REGISTRO_COMPRA + 1];
    temp[0] = '\0';
    strcpy(temp, c.id_user_dono);
    strcat(temp, c.data_compra);
    strcat(temp, c.id_game);
    //escrevemos a compra no arquivo
    strncpy(ARQUIVO_COMPRAS + rrn*TAM_REGISTRO_COMPRA, temp, TAM_REGISTRO_COMPRA);
    ARQUIVO_COMPRAS[qtd_registros_compras*TAM_REGISTRO_COMPRA] = '\0';
}

//todo
/* Funções principais */
void cadastrar_usuario_menu(char *id_user, char *username, char *email) {
   if(btree_search(NULL, false, id_user, usuarios_idx.rrn_raiz, &usuarios_idx )){
       printf(ERRO_PK_REPETIDA, id_user);
       return;
   }

    //criamos o usuário
    Usuario u;
    strcpy(u.id_user, id_user);
    strcpy(u.username, username);
    strcpy(u.email, email);
    strcpy(u.celular, "***********");
    u.saldo = 0;

    //adcionamos o usuário no registro, e adcionamo-os no índice
    escrever_registro_usuario(u, qtd_registros_usuarios++);
    add_usuario_idx();
    printf(SUCESSO);
}

void cadastrar_celular_menu(char* id_user, char* celular) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    //verificamos se o usuário existe
    char resultado_busca[usuarios_idx.tam_chave+1];
    resultado_busca[0] = '\0';
    bool chave_encontrada = btree_search(resultado_busca, false, id_user, usuarios_idx.rrn_raiz, &usuarios_idx);
    if(!chave_encontrada){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
    char* str_rrn = resultado_busca+ TAM_ID_USER;
    int rrn = atoi(str_rrn);
//    recuperamos o registro de usuário, alteramos seu saldo, e salvamos
    Usuario u = recuperar_registro_usuario(rrn);
    strcpy(u.celular, celular);
    escrever_registro_usuario(u, rrn);
    printf(SUCESSO);

}

void cadastrar_jogo_menu(char *titulo, char *desenvolvedor, char *editora, char* lancamento, double preco) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char str_titulo[TAM_MAX_TITULO];
    str_titulo[0] = '\0';
    strcat(str_titulo, titulo);
    strpadright(str_titulo, '#',TAM_MAX_TITULO-1);
    if(btree_search(NULL, false, str_titulo, titulo_idx.rrn_raiz, &titulo_idx )){
        printf(ERRO_PK_REPETIDA, titulo);
        return;
    }
    Jogo j;
    sprintf(j.id_game, "%08d", qtd_registros_jogos);
    strcpy(j.titulo, titulo);
    strcpy(j.desenvolvedor, desenvolvedor);
    strcpy(j.editora, editora);
    strcpy(j.data_lancamento, lancamento);
    j.preco = preco;
    //iniciamos as categorias como vazias
    strcpy(j.categorias[0], "\0");
    strcpy(j.categorias[1], "\0");
    strcpy(j.categorias[2], "\0");
    //salvamos o jogo no arquivo e atualizamos os indexes de jogo
    escrever_registro_jogo(j, qtd_registros_jogos++);
    add_id_jogo_idx();
    add_titulo_idx();
    printf(SUCESSO);
}

void adicionar_saldo_menu(char *id_user, double valor) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    //Verificamos se o valor é negativo
    if(valor <= 0) {
        printf(ERRO_VALOR_INVALIDO);
        return;
    }
    char resultado_busca[usuarios_idx.tam_chave+1];
    strcpy(resultado_busca, "");
    bool chave_encontrada = btree_search(resultado_busca, false, id_user, usuarios_idx.rrn_raiz, &usuarios_idx);
    if(!chave_encontrada){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
    char* str_rrn = resultado_busca + TAM_ID_USER;
    int rrn = atoi(str_rrn);
//    recuperamos o registro de usuário, alteramos seu saldo, e salvamos
    Usuario u = recuperar_registro_usuario(rrn);
    u.saldo += valor;
    escrever_registro_usuario(u, rrn);
    printf(SUCESSO);

}

void comprar_menu(char *id_user, char *titulo) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char resultado_user[TAM_CHAVE_USUARIOS_IDX+1];
    if (!btree_search(resultado_user, false, id_user, usuarios_idx.rrn_raiz, &usuarios_idx)) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
    char str_titulo[TAM_MAX_TITULO];
    str_titulo[0] = '\0';
    strcat(str_titulo, titulo);
    strpadright(str_titulo, '#',TAM_MAX_TITULO-1);
    char resultado_titulo[TAM_CHAVE_TITULO_IDX+1];

    if (!btree_search(resultado_titulo, false, str_titulo, titulo_idx.rrn_raiz, &titulo_idx)) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }
    char id_game[TAM_CHAVE_JOGOS_IDX];
    strcpy(id_game, resultado_titulo+TAM_MAX_TITULO);
    char resultado_id_game[TAM_ID_GAME];
    btree_search(resultado_id_game, false, id_game, jogos_idx.rrn_raiz, &jogos_idx);

    //usuario ou jogo não existe
    int rrn_jogo = atoi(resultado_id_game+TAM_ID_GAME);
    Jogo j = recuperar_registro_jogo(rrn_jogo);
    int rrn_user = atoi(resultado_user+TAM_ID_USER);
    Usuario u = recuperar_registro_usuario(rrn_user);

    //Verificamos se a compra já existe

    char chave_compra[TAM_CHAVE_COMPRAS_IDX+1];
    strcpy(chave_compra, u.id_user);
    strcat(chave_compra, j.id_game);
    if(btree_search(NULL, false, chave_compra, compras_idx.rrn_raiz, &compras_idx)){
        printf(ERRO_PK_REPETIDA, chave_compra);
        return;
    }


    if(u.saldo < j.preco) {
        printf(ERRO_SALDO_NAO_SUFICIENTE);
        return;
    }
    //subtraimos o valor do jogo do saldo do usuario
    u.saldo -= j.preco;
    //salvamos o novo saldo do usuario
    escrever_registro_usuario(u, rrn_user);

    //criamos a compra e a salvamos
    Compra c;
    strcpy(c.id_user_dono, u.id_user);
    strcpy(c.id_game, j.id_game);
    char data[TAM_DATE];
    current_date(data);
    strcpy(c.data_compra, data);

    //salvamos a compra no arquivo e atualizamos os indices de compras
    escrever_registro_compra(c, qtd_registros_compras++);
    //todo
    add_compra_idx();
    add_data_user_game_idx();
    printf(SUCESSO);
}

void cadastrar_categoria_menu(char* titulo, char* categoria) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_categoria_menu");
}


/* Busca */
void buscar_usuario_id_user_menu(char *id_user) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char resultado[TAM_CHAVE_USUARIOS_IDX+1];
    if (btree_search(resultado, true, id_user, usuarios_idx.rrn_raiz, &usuarios_idx)) {
        exibir_btree_usuario(resultado);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void buscar_jogo_id_menu(char *id_game) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char resultado[TAM_CHAVE_JOGOS_IDX];
    if (btree_search(resultado, true, id_game, jogos_idx.rrn_raiz, &jogos_idx)) {
        exibir_btree_jogo(resultado);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void buscar_jogo_titulo_menu(char *titulo) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char str_titulo[TAM_MAX_TITULO];
    str_titulo[0] = '\0';
    strcat(str_titulo, titulo);
    strpadright(str_titulo, '#',TAM_MAX_TITULO-1);

    char resultado[TAM_CHAVE_TITULO_IDX+1];
    if (btree_search(resultado, true, str_titulo, titulo_idx.rrn_raiz, &titulo_idx)) {
        buscar_jogo_id_menu(resultado+TAM_MAX_TITULO-1);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}


/* Listagem */
void listar_usuarios_id_user_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    btree_print_in_order(NULL, NULL, &exibir_btree_usuario, usuarios_idx.rrn_raiz, &usuarios_idx);
}

void listar_jogos_categorias_menu(char *categoria) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "listar_jogos_categorias_menu");
}

void listar_compras_periodo_menu(char *data_inicio, char *data_fim) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(!btree_print_in_order(data_inicio, data_fim, &exibir_btree_data_user_game, data_user_game_idx.rrn_raiz, &data_user_game_idx)){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
    } else {
        printf("\n");
    }
}


/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_USUARIOS);
}

void imprimir_arquivo_jogos_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_JOGOS);
}

void imprimir_arquivo_compras_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_COMPRAS);
}


/* Imprimir arquivos de índice */
void imprimir_usuarios_idx_menu() {
    if (usuarios_idx.qtd_nos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_USUARIOS_IDX);
}

void imprimir_jogos_idx_menu() {
    if (jogos_idx.qtd_nos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_JOGOS_IDX);
}

void imprimir_compras_idx_menu() {
    if (compras_idx.qtd_nos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_COMPRAS_IDX);
}

void imprimir_titulo_idx_menu() {
    if (titulo_idx.qtd_nos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_TITULO_IDX);
}

void imprimir_data_user_game_idx_menu() {
    if (data_user_game_idx.qtd_nos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_DATA_USER_GAME_IDX);
}

void imprimir_categorias_secundario_idx_menu() {
    if (categorias_idx.qtd_registros_secundario == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else    
        printf("%s\n", ARQUIVO_CATEGORIAS_SECUNDARIO_IDX);
}

void imprimir_categorias_primario_idx_menu() {
    if (categorias_idx.qtd_registros_primario == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_CATEGORIAS_PRIMARIO_IDX);
}


/* Função de comparação entre chaves do índice usuarios_idx */
int order_usuarios_idx(const void *key, const void *elem) {
    return strncmp(key, elem, TAM_ID_USER - 1);
}

/* Função de comparação entre chaves do índice jogos_idx */
int order_jogos_idx(const void *key, const void *elem) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strncmp(key, elem, TAM_ID_GAME - 1);
}

/* Função de comparação entre chaves do índice compras_idx */
int order_compras_idx(const void *key, const void *elem) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    int tam_chave = TAM_ID_USER+TAM_ID_GAME;
    return strncmp(key, elem, tam_chave-2);
}

/* Função de comparação entre chaves do índice titulo_idx */
int order_titulo_idx(const void *key, const void *elem) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strncmp(key, elem, TAM_MAX_TITULO - 1);
}

/* Função de comparação entre chaves do índice data_user_game_idx */
int order_data_user_game_idx(const void *key, const void *elem) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strncmp(key, elem, TAM_DATE-1);
}

/* Função de comparação entre chaves do índice secundário de categorias_idx */
int order_categorias_idx(const void *key, const void *elem) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "order_categorias_idx");
    return -1;
}


/* Funções de manipulação de Lista Invertida */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_insert");
}

bool inverted_list_secondary_search(int* result, bool exibir_caminho, char *chave, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_secondary_search");
    return false;
}

int inverted_list_primary_search(int* rrn_final, char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int rrn, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_primary_search");
    return -1;
}

bool inverted_list_binary_search(int* result, bool exibir_caminho, char *chave, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_binary_search");
    return false;
}


/* Funções de manipulação de Árvores-B */
void btree_insert(char *chave, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    //arvore vazia
    if(t->qtd_nos == 0) {
        t->rrn_raiz = 0;
        t->qtd_nos = 1;
        
        btree_node no =  btree_node_malloc(t);
        no.this_rrn = 0;
        no.folha = true;
        no.qtd_chaves = 1;
        
        strcpy(no.chaves[0], chave);
        
        btree_write(no, t);
        btree_node_free(no);
        return;
    }
    
    //arvore não vazia
    promovido_aux a =  btree_insert_aux(chave, t->rrn_raiz, t);
    if(strlen(a.chave_promovida) != 0) {
        //TODO ->dar uma verificada quanto a ter um no cheio
        t->qtd_nos++;
        btree_node no = btree_node_malloc(t);
        no.folha = false;
        no.this_rrn = t->qtd_nos-1;
        no.qtd_chaves = 1;
        strcpy(no.chaves[0], a.chave_promovida);
        no.filhos[0] = t->rrn_raiz;
        no.filhos[1] = a.filho_direito;
        t->rrn_raiz = no.this_rrn;
        btree_write(no, t);
        btree_node_free(no);
    }
}

promovido_aux btree_insert_aux(char *chave, int rrn, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
   btree_node no = btree_read(rrn, t);
   //no folha
   if(no.folha){
       //no folha com espaço
       if(no.qtd_chaves < btree_order-1){
           int i = no.qtd_chaves - 1;
           while(i>=0 && t->compar(chave, no.chaves[i])<0) {
               char vouCopiar[t->tam_chave + 1];
               strcpy(vouCopiar, no.chaves[i]);
               strcpy(no.chaves[i+1], no.chaves[i]);
               i--;
           }
           strcpy(no.chaves[i+1], chave);
           no.qtd_chaves++;

           btree_write(no, t);
           btree_node_free(no);

           promovido_aux a;
           a.chave_promovida[0] = '\0';
           a.filho_direito = -1;
           return a;
       }
       btree_node_free(no);
       //folha sem espaço
       return btree_divide(chave, -1, rrn, t);
   }
   //nó não folha
   int i = no.qtd_chaves-1;

   while(i>=0 && t->compar(chave, no.chaves[i]) < 0) {
       i--;
   }
   i++;
   promovido_aux a = btree_insert_aux(chave, no.filhos[i], t);

   if(strlen(a.chave_promovida) != 0) {
       chave = a.chave_promovida;
       if(no.qtd_chaves < btree_order-1) {
           i = no.qtd_chaves-1;
           while(i>=0 && t->compar(chave, no.chaves[i]) < 0) {
               if(i == btree_order-2){
                   i--;
                   break;
               }
               //ponto de atenção nesse i+1 -> sigfault
               strcpy(no.chaves[i+1], no.chaves[i]);
               //ponto de atenção qui tbm nesse +2
               no.filhos[i+2] = no.filhos[i+1];
               i--;
           }
           //cuidado com o sig dnovo
           strcpy(no.chaves[i+1], chave);
           no.filhos[i+2] = a.filho_direito;
           no.qtd_chaves++; //-> não entendi direito essa parte

           a.chave_promovida[0] = '\0';
           a.filho_direito = -1;
           btree_write(no, t);
           btree_node_free(no);
           return a;
       }
       else {
           int rrn = no.this_rrn;
           btree_node_free(no);
           return btree_divide(chave, a.filho_direito, rrn, t);
       }
   }
   else {
       btree_node_free(no);
       a.chave_promovida[0] = '\0';
       a.filho_direito = -1;
       return a;
   }
}

promovido_aux btree_divide(char *chave, int filho_direito, int rrn, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    btree_node no_x = btree_read(rrn, t);
    btree_node no_y = btree_node_malloc(t);
    t->qtd_nos++;
    no_y.this_rrn = t->qtd_nos-1;
    no_y.qtd_chaves = (btree_order-1)/2;

    int i = no_x.qtd_chaves - 1;
    bool chave_alocada = false;

    //percorremos o nó Y
    for (int j = no_y.qtd_chaves-1; j >= 0 ; --j)
    {   //chave não alocada e maior do que X[i]
        if(!chave_alocada && t->compar(chave, no_x.chaves[i]) > 0) {
            //copiamos a chave para Y[j]
            strcpy(no_y.chaves[j], chave);
            //alocamos o filho direito
            no_y.filhos[j+1] = filho_direito;
            chave_alocada = true;
        }
        else  {
            //copiamos de X[i] para Y[j]
            strcpy(no_y.chaves[j], no_x.chaves[i]);
            //TODO
            strcpy(no_x.chaves[i], "");

//            no_x.chaves[i][0] = '\0';
            no_y.filhos[j+1] =  no_x.filhos[i+1];
            i--;
        }
    }
    //chave ainda não alocada
    if(!chave_alocada)
    {
        //enquanto não for percorrido t odo o nó X e a chave for maior
        while(i>=0 && t->compar(chave, no_x.chaves[i]) < 0) {
            //copiamos de X[i+1] para X[i] -> movemos as chaves para a direita
            strcpy(no_x.chaves[i+1], no_x.chaves[i]);
            //movemos os fihos para a direita
            no_x.filhos[i+2] = no_x.filhos[i+1];
            i--;
        }
        //salvamos a chave e o filho
        strcpy(no_x.chaves[i+1], chave);
        no_x.filhos[i+2] = filho_direito;
    }
    promovido_aux a;
    //TODO
    //caso especifico para ordem 3 trocar depois

    strcpy(a.chave_promovida, no_x.chaves[(btree_order/2)]);
    a.filho_direito = no_y.this_rrn;
    no_y.folha = no_x.folha;
    no_x.chaves[(btree_order/2)][0] = '\0';
    no_y.filhos[0] = no_x.filhos[(btree_order/2)+1];
    no_x.filhos[(btree_order/2)+1] = -1;
    no_x.qtd_chaves = btree_order/2;

    btree_write(no_y, t);
    btree_write(no_x, t);
    btree_node_free(no_y);
    btree_node_free(no_x);
    return a;
}

bool btree_search(char *result, bool exibir_caminho, char *chave, int rrn, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(exibir_caminho){
        if(rrn == t->rrn_raiz) {
            printf(RRN_NOS);
        }
        printf(" %d (", rrn);
    }
    if(t->qtd_nos == 0) {
        return false;
    }
    btree_node node = btree_read(rrn, t);
    int resultado;
    //TODO o problema está aqui
    bool chave_encontrada =  btree_binary_search(&resultado, exibir_caminho, chave, &node, t);


    if(chave_encontrada) {
        if(result) {
            strcpy(result, node.chaves[resultado]);
        }
        btree_node_free(node);
        if(exibir_caminho){
            printf("\n");
        }
        return true;
    }
    else {
        int rrn = node.filhos[resultado];
        bool folha = node.folha;
        btree_node_free(node);
        if(folha) {
            if(exibir_caminho){
                printf("\n");
            }
            return false;
        }

        bool retorno = btree_search(result, exibir_caminho,chave, rrn, t);
        if(exibir_caminho){
            if(rrn == t->rrn_raiz) {
                printf("\n");
            }
        }
        return retorno;
    }
}

bool btree_binary_search(int *result, bool exibir_caminho, char* chave, btree_node* node, btree* t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    int index = 0;
    char chave_atual[t->tam_chave+1];
    int posicao;

    for (int lim = node->qtd_chaves; lim > 0 ; lim = lim/2) {
        posicao = index + lim/2;
        if(exibir_caminho){
            if(lim == node->qtd_chaves){
                printf("%d", posicao);
            }
            else{
                printf(" %d", posicao);
            }
        }


//        index += lim/2;
        strcpy(chave_atual, node->chaves[posicao]);
//        chave_atual = node->chaves[posicao];
        int comp = t->compar(chave, node->chaves[posicao]);

        //valor procurado
        if(comp == 0) {
            *result = posicao;
            if(exibir_caminho){
                printf(")");
            }
            return true;
        }

        if(comp > 0) {//move para a direita - valor menor
            index += (lim/2) +1;
            *result =  posicao+1;
            lim--;
        }
        else {
            *result =  posicao;
        }
        //move pra esquerda
    }
    if(exibir_caminho){
        printf(")");
    }
    return false;
}

bool btree_print_in_order(char *chave_inicio, char *chave_fim, bool (*exibir)(char *chave), int rrn, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    btree_node no =  btree_read(rrn, t);
    int n_impressoes = 0;
    for (int i = 0; i <= no.qtd_chaves-1; ++i) {
        if(no.filhos[i] != -1){
            bool houve_impressao = btree_print_in_order(chave_inicio, chave_fim, exibir, no.filhos[i], t);
            if(houve_impressao){
                n_impressoes++;
            }
        }
        if(chave_inicio == NULL || (t->compar(no.chaves[i], chave_inicio)>=0 && t->compar(no.chaves[i], chave_fim)<= 0)){
            exibir(no.chaves[i]);
            n_impressoes++;
        }
    }
    if(no.filhos[no.qtd_chaves] != -1){
        if(btree_print_in_order(chave_inicio, chave_fim, exibir, no.filhos[no.qtd_chaves], t)){
            n_impressoes++;
        }
    }
    btree_node_free(no);
    if(n_impressoes == 0){
        return false;
    }
    return true;
}

btree_node btree_read(int rrn, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    btree_node no = btree_node_malloc(t);
    char temp[btree_register_size(t) + 1], aux[btree_register_size(t)], *p;
    p = temp;

    //copiamos do arquivo para a String temporaria
    strncpy(temp, t->arquivo + (rrn * btree_register_size(t)), btree_register_size(t));
    temp[btree_register_size(t)] = '\0';

    strncpy(aux, p, 3);
    aux[3] = '\0';//aqui

    no.qtd_chaves = atoi(aux);
    p += 3;

    //chaves
    for (int i = 0; i < btree_order-1; ++i) {
        if(p[0] != '#') {
            char chaveTemp[t->tam_chave + 1];
            strncpy(chaveTemp, p, t->tam_chave);
            chaveTemp[t->tam_chave] = '\0';
            strcpy(no.chaves[i], chaveTemp);
        }
        p += t->tam_chave;
    }

    //folha ou não
    if(p[0] == 'F'){
        no.folha = false;
    }
    else {
        no.folha = true;
    }
    p += 1;
    //filhos
    for (int i = 0; i < btree_order; ++i) {
        if(p[0] != '*') {
            strncpy(aux, p, 3);
            no.filhos[i] = atoi(aux);
        }
        p += 3;
    }
    no.this_rrn = rrn;
    return no;
}

void btree_write(btree_node no, btree *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    //criamos a String temporaria para armazena o nó
    char temp[btree_register_size(t) + 1], p[10];
    temp[0] = '\0'; p[0] = '\0';

    //quantida de chaves
    sprintf(p, "%03d", no.qtd_chaves);
    strcpy(temp, p);

    //escrevemos as chaves do nó
    for (int i = 0; i < btree_order-1; ++i) {
        //chave vazia colocamos ####
        if(strlen(no.chaves[i]) == 0) {
            char vazio[t->tam_chave+1];
            vazio[0] = '\0';
            strpadright(vazio, '#', t->tam_chave);
            strcat(temp, vazio);
        }
        //possui chave, apenas copiamos
        else
            strcat(temp, no.chaves[i]);
    }
//   se for nó folha, marcamos como TRUE, colocamos os ponteiros dos filhos com nulos (***)
    if(no.folha) {
        strcat(temp, "T");
        //ponteiro para os filhos  nulos
        for (int i = 0; i < btree_order; ++i) {
            strcat(temp, "***");
        }
    }
    //se não for folha é marcado como falso, verificamos os ponteiros para os filhos
    else {
        strcat(temp, "F");
        //ponteiro nó filhos
        for (int i = 0; i < btree_order; ++i) {
            if(no.filhos[i] == -1) {
                strcat(temp, "***");
            }
            else {
                p[0] = '\0';
                sprintf(p, "%03d", no.filhos[i]);
                strcat(temp, p);
            }
        }
    }
    //copiamos a String temporaria para o arquivo da arvore
    strncpy(t->arquivo + (no.this_rrn * btree_register_size(t)), temp, btree_register_size(t));
    t->arquivo[qtd_registros_usuarios * btree_register_size(t)] = '\0';
}

int btree_register_size(btree *t) {
    int chaves_ordenadas = (btree_order-1)*t->tam_chave;
    return 3 + chaves_ordenadas + 1 + (btree_order*3);
}

btree_node btree_node_malloc(btree *t) {
    btree_node no;

    no.chaves = malloc((btree_order-1) * sizeof(char*));
    for (int i = 0; i < btree_order-1; ++i) {
        no.chaves[i] = malloc(t->tam_chave+1);
        no.chaves[i][0] = '\0';
    }

    no.filhos = malloc(btree_order * sizeof(int));
    for (int i = 0; i < btree_order; ++i)
        no.filhos[i] = -1;

    return no;
}

void btree_node_free(btree_node no) {
    for (int i = 0; i < btree_order-1; ++i)
        free(no.chaves[i]);

    free(no.chaves);
    free(no.filhos);
}

char* strpadright(char *str, char pad, unsigned size) {
    for (unsigned i = strlen(str); i < size; ++i)
        str[i] = pad;
    str[size] = '\0';
    return str;
}

void add_usuario_idx() {
    char usuario_str[TAM_CHAVE_USUARIOS_IDX + 1];

    Usuario u = recuperar_registro_usuario(qtd_registros_usuarios-1);

    sprintf(usuario_str, "%s%04d", u.id_user, qtd_registros_usuarios-1);
    btree_insert(usuario_str, &usuarios_idx);
}

void add_id_jogo_idx() {
    char jogo_str[TAM_CHAVE_JOGOS_IDX + 1];

    Jogo j = recuperar_registro_jogo(qtd_registros_jogos-1);

    sprintf(jogo_str, "%s%04d", j.id_game, qtd_registros_jogos-1);
    btree_insert(jogo_str, &jogos_idx);
}

void add_compra_idx() {
    char compra_str[TAM_CHAVE_COMPRAS_IDX + 1];

    Compra c = recuperar_registro_compra(qtd_registros_compras-1);

    sprintf(compra_str, "%s%s%04d",c.id_user_dono, c.id_game, qtd_registros_compras-1);
    btree_insert(compra_str, &compras_idx);
}

void add_titulo_idx() {
    char titulo_str[TAM_CHAVE_TITULO_IDX + 1];
    char titulo_jogo[TAM_MAX_TITULO-1];

    Jogo j = recuperar_registro_jogo(qtd_registros_jogos-1);
    strcpy(titulo_jogo, j.titulo);
    strpadright(titulo_jogo, '#', TAM_MAX_TITULO-1);
    sprintf(titulo_str, "%s%s", titulo_jogo, j.id_game);
    btree_insert(titulo_str, &titulo_idx);

}

void add_data_user_game_idx() {
    char compra_str[TAM_CHAVE_DATA_USER_GAME_IDX + 1];

    Compra c = recuperar_registro_compra(qtd_registros_compras-1);

    sprintf(compra_str, "%s%s%s", c.data_compra, c.id_user_dono, c.id_game);
    btree_insert(compra_str, &data_user_game_idx);
}