
#include <iostream>
#include <string>
#include <sqlite3.h>

using namespace std;

sqlite3* db = nullptr;

void executarSQL(const string& sql) {
    char* erro = nullptr;

    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &erro) != SQLITE_OK) {
        cerr << "Erro SQL: " << erro << '\n';
        sqlite3_free(erro);
    }
}

void consultar(const string& sql) {
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Erro na consulta: " << sqlite3_errmsg(db) << '\n';
        return;
    }

    int colunas = sqlite3_column_count(stmt);

    for (int i = 0; i < colunas; ++i) {
        cout << sqlite3_column_name(stmt, i) << "\t";
    }
    cout << "\n----------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int i = 0; i < colunas; ++i) {
            const unsigned char* valor = sqlite3_column_text(stmt, i);
            cout << (valor ? reinterpret_cast<const char*>(valor) : "NULL")
                 << "\t";
        }
        cout << '\n';
    }

    sqlite3_finalize(stmt);
}

void prepararCaso() {
    executarSQL(R"(
        CREATE TABLE IF NOT EXISTS suspeitos (
            id INTEGER PRIMARY KEY,
            nome TEXT NOT NULL,
            cargo TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS sessoes (
            id TEXT PRIMARY KEY,
            usuario_real_id INTEGER NOT NULL,
            terminal TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS acessos (
            id INTEGER PRIMARY KEY,
            conta_utilizada TEXT NOT NULL,
            sessao_id TEXT NOT NULL,
            horario TEXT NOT NULL,
            arquivo TEXT NOT NULL,
            acao TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS registros_fisicos (
            id INTEGER PRIMARY KEY,
            suspeito_id INTEGER NOT NULL,
            local TEXT NOT NULL,
            horario TEXT NOT NULL
        );

        INSERT OR IGNORE INTO suspeitos VALUES
            (1, 'Lara Mendes', 'Analista de sistemas'),
            (2, 'Bruno Costa', 'Administrador de dados'),
            (3, 'Caio Nunes', 'Suporte tecnico');

        INSERT OR IGNORE INTO sessoes VALUES
            ('S-77', 1, 'TERMINAL-03'),
            ('S-81', 3, 'TERMINAL-02');

        INSERT OR IGNORE INTO acessos VALUES
            (1, 'bruno.costa', 'S-77', '22:14',
             'projeto_orion.confidencial', 'DOWNLOAD'),
            (2, 'caio.nunes', 'S-81', '21:42',
             'relatorio_publico.pdf', 'LEITURA');

        INSERT OR IGNORE INTO registros_fisicos VALUES
            (1, 1, 'Sala de servidores', '22:10'),
            (2, 2, 'Saida do edificio', '22:05'),
            (3, 3, 'Copa', '22:12');
    )");
}

void menu() {
    cout << "\n========== ARQUIVO 026 ==========\n";
    cout << "1. Consultar suspeitos\n";
    cout << "2. Examinar registros de acesso\n";
    cout << "3. Investigar sessoes e identidades\n";
    cout << "4. Examinar registros fisicos\n";
    cout << "5. Apresentar acusacao\n";
    cout << "0. Encerrar investigacao\n";
    cout << "Escolha: ";
}

int main() {
    if (sqlite3_open("caso.db", &db) != SQLITE_OK) {
        cerr << "Nao foi possivel abrir o banco de dados.\n";
        return 1;
    }

    prepararCaso();

    int opcao = -1;

    cout << "ARQUIVO 026: O ULTIMO ACESSO\n";
    cout << "Um arquivo confidencial foi baixado as 22:14.\n";
    cout << "Descubra quem estava por tras do acesso.\n";

    while (opcao != 0) {
        menu();

        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Digite uma opcao valida.\n";
            continue;
        }

        switch (opcao) {
            case 1:
                consultar(
                    "SELECT id, nome, cargo FROM suspeitos;"
                );
                break;

            case 2:
                consultar(
                    "SELECT horario, conta_utilizada, arquivo, acao "
                    "FROM acessos ORDER BY horario;"
                );
                break;

            case 3:
                consultar(
                    "SELECT a.horario, a.conta_utilizada, "
                    "s.usuario_real_id, p.nome AS dono_da_sessao, "
                    "s.terminal "
                    "FROM acessos a "
                    "JOIN sessoes s ON a.sessao_id = s.id "
                    "JOIN suspeitos p ON s.usuario_real_id = p.id;"
                );
                break;

            case 4:
                consultar(
                    "SELECT p.nome, r.local, r.horario "
                    "FROM registros_fisicos r "
                    "JOIN suspeitos p ON r.suspeito_id = p.id "
                    "ORDER BY r.horario;"
                );
                break;

            case 5: {
                int acusado;

                cout << "ID do suspeito (1, 2 ou 3): ";
                if (!(cin >> acusado)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "ID invalido.\n";
                    break;
                }

                if (acusado == 1) {
                    cout << "\nACUSACAO ACEITA!\n";
                    cout << "Lara usou a conta de Bruno "
                            "na sessao S-77.\n";
                    cout << "A sessao estava associada a ela, "
                            "e o registro fisico a coloca "
                            "na sala de servidores.\n";
                } else if (acusado == 2 || acusado == 3) {
                    cout << "\nEvidencias insuficientes.\n";
                    cout << "Cruze a conta utilizada com "
                            "a identidade dona da sessao.\n";
                } else {
                    cout << "Suspeito inexistente.\n";
                }
                break;
            }

            case 0:
                cout << "Investigacao encerrada.\n";
                break;

            default:
                cout << "Opcao inexistente.\n";
        }
    }

    sqlite3_close(db);
    return 0;
}
