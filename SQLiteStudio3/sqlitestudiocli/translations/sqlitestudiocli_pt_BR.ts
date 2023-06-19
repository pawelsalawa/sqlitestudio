<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pt-BR" sourcelanguage="en">
  <context>
    <name>CLI</name>
    <message>
      <location filename="../cli.cpp" line="98"/>
      <source>Current database: %1</source>
      <translation>Banco de dados atual: %1</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="100"/>
      <source>No current working database is set.</source>
      <translation>Nenhuma banco de dados de trabalho atual está definido.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="102"/>
      <source>Type %1 for help</source>
      <translation>Digite %1 para ajuda</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="254"/>
      <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
      <translation>Banco de dados passado nos parâmetros da linha de comando (%1) já estava na lista com o nome: %2</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="262"/>
      <source>Could not add database %1 to list.</source>
      <translation>Não foi possível adicionar o banco de dados %1 à lista.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="289"/>
      <source>closed</source>
      <translation>fechado</translation>
    </message>
  </context>
  <context>
    <name>CliCommand</name>
    <message>
      <location filename="../commands/clicommand.cpp" line="107"/>
      <source>Usage: %1%2</source>
      <translation>Uso: %1%2</translation>
    </message>
  </context>
  <context>
    <name>CliCommandAdd</name>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="9"/>
      <source>Could not add database %1 to list.</source>
      <translation>Não foi possível adicionar o banco de dados %1 à lista.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="14"/>
      <source>Database added: %1</source>
      <translation>Banco de dados adicionado: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="19"/>
      <source>adds new database to the list</source>
      <translation>adiciona novo banco de dados à lista</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="24"/>
      <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
      <translation>Adiciona um banco de dados apontado por &lt;path&gt; com determinado &lt;name&gt; para listar a lista de bancos de dados. O &lt;name&gt; é apenas um nome simbólico que você pode referir mais tarde. Escolha qualquer nome exclusivo. Para lista de bancos de dados já estão na lista use o comando %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="34"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="35"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>caminho</translation>
    </message>
  </context>
  <context>
    <name>CliCommandCd</name>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="10"/>
      <source>Changed directory to: %1</source>
      <translation>Diretório alterado para: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="12"/>
      <source>Could not change directory to: %1</source>
      <translation>Não foi possível mudar o diretório para: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="17"/>
      <source>changes current working directory</source>
      <translation>muda o diretório atual de trabalho</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="22"/>
      <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
      <translation>Um comando muito semelhante ao &apos;cd&apos; conhecido do Unix system e Windows. É necessário que um argumento &lt;path&gt; seja aprovado, portanto chamar %1 sempre causará uma mudança do diretório. Para saber qual diretório de trabalho atual usa o comando %2 e para listar o conteúdo do diretório de trabalho atual use o comando %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="33"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>caminho</translation>
    </message>
  </context>
  <context>
    <name>CliCommandClose</name>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="10"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Não é possível chamar %1 quando nenhum banco de dados está definido como atual. Especifique o banco de dados atual com o comando %2 ou passe o nome do banco de dados para %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="21"/>
      <location filename="../commands/clicommandclose.cpp" line="29"/>
      <source>Connection to database %1 closed.</source>
      <translation>Conexão ao banco de dados %1 fechado.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="24"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Nenhum banco de dados: %1. Use %2 para ver a lista de bancos de dados conhecidos.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="35"/>
      <source>closes given (or current) database</source>
      <translation>fecha o banco de dados</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="40"/>
      <source>Closes the database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be the name of the database to close (as printed by the %1 command). If &lt;name&gt; is not provided, then the current working database is closed (see help for %2 for details).</source>
      <translation type="unfinished">Closes the database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be the name of the database to close (as printed by the %1 command). If &lt;name&gt; is not provided, then the current working database is closed (see help for %2 for details).</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDbList</name>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="12"/>
      <source>No current working database defined.</source>
      <translation>Nenhuma banco de dados de trabalho atual definido.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="18"/>
      <source>Databases:</source>
      <translation>Banco de dados:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="23"/>
      <location filename="../commands/clicommanddblist.cpp" line="34"/>
      <source>Name</source>
      <comment>CLI db name column</comment>
      <translation>Nome</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Open</source>
      <comment>CLI connection state column</comment>
      <translation>Abrir</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Closed</source>
      <comment>CLI connection state column</comment>
      <translation>Fechado</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="32"/>
      <location filename="../commands/clicommanddblist.cpp" line="36"/>
      <source>Connection</source>
      <comment>CLI connection state column</comment>
      <translation>Conexão</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="38"/>
      <location filename="../commands/clicommanddblist.cpp" line="45"/>
      <source>Database file path</source>
      <translation>Caminho do arquivo de banco de dados</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="70"/>
      <source>prints list of registered databases</source>
      <translation>lista de bancos de dados registrados</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="75"/>
      <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
      <translation>Mostra a lista de bancos de dados registrados no SQLiteStudio. Cada banco de dados da lista pode ser aberto ou fechado %1 avisa isso. O banco de dados de trabalho atual (conhecido como padrão de banco de dados) também está marcado na lista com &apos;*&apos; no início do seu nome. Consulte ajuda para usar o comando %2 para aprender sobre o banco de dados padrão.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDesc</name>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="15"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Nenhum banco de dados está definido.
Use %1 para definir o banco de dados ativo.
Use %2 para ver a lista de todos os bancos de dados.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="26"/>
      <source>Database is not open.</source>
      <translation>Banco de dados não está aberto.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="35"/>
      <source>Cannot find table named: %1</source>
      <translation>Não foi possível encontrar a tabela: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="52"/>
      <source>shows details about the table</source>
      <translation>mostra detalhes sobre a tabela</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="63"/>
      <source>table</source>
      <translation>tabela</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="70"/>
      <source>Table: %1</source>
      <translation>Tabela: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="74"/>
      <source>Column name</source>
      <translation>Nome da coluna</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="76"/>
      <source>Data type</source>
      <translation>Tipo de dado</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="80"/>
      <source>Constraints</source>
      <translation>Restrições</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="105"/>
      <source>Virtual table: %1</source>
      <translation>Tabela virtual: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="109"/>
      <source>Construction arguments:</source>
      <translation>Argumentos de construção:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="114"/>
      <source>No construction arguments were passed for this virtual table.</source>
      <translation>Não foram apresentados argumentos de construção para esta tabela virtual.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDir</name>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="33"/>
      <source>lists directories and files in current working directory</source>
      <translation>lista diretórios e arquivos no diretório de trabalho atual</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="38"/>
      <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
      <translation>Isso é muito semelhante ao comando &apos;dir&apos; do Windows e &apos;ls&apos; do sistema Unix.

Você pode passar &lt;pattern&gt; como caracteres curinga para filtrar a saída.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="49"/>
      <source>pattern</source>
      <translation>padrão</translation>
    </message>
  </context>
  <context>
    <name>CliCommandExit</name>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="12"/>
      <source>quits the application</source>
      <translation>sair da aplicação</translation>
    </message>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="17"/>
      <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
      <translation>Encerra o aplicativo. As configurações são armazenadas no arquivo de configuração e serão restauradas na próxima inicialização.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHelp</name>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="16"/>
      <source>shows this help message</source>
      <translation>mostra mensagem de ajuda</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="21"/>
      <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
      <translation>Use %1 para aprender sobre certos comandos suportados pela interface de linha de comando (CLI) do SQLiteStudio.
Para ver a lista de comandos suportados, digite %2 sem quaisquer argumentos.

Ao passar o nome &lt;command&gt; você pode pular o caractere de prefixo especial (&apos;%3&apos;).

Você sempre pode executar qualquer comando com exatamente um único &apos;--help&apos; opção para ver a ajuda para esse comando. Uma alternativa para digitar: %1 &lt;command&gt;.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="33"/>
      <source>command</source>
      <comment>CLI command syntax</comment>
      <translation>comando</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="42"/>
      <source>No such command: %1</source>
      <translation>Comando não encontrado: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="43"/>
      <source>Type &apos;%1&apos; for list of available commands.</source>
      <translation>Digite &apos;%1&apos; para a lista de comandos disponíveis.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="52"/>
      <source>Usage: %1%2</source>
      <translation>Uso: %1%2</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="62"/>
      <source>Aliases: %1</source>
      <translation>Apelidos: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHistory</name>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="23"/>
      <source>Current history limit is set to: %1</source>
      <translation>Limite do histórico atual está definido para: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="39"/>
      <source>prints history or erases it</source>
      <translation>mostrar ou apagar o histórico</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="44"/>
      <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
      <translation>Quando nenhum argumento foi utilizado, este comando imprime o histórico da linha de comando. Cada entrada do histórico é separada por uma linha horizontal, então as entradas multilinha são mais fáceis de ler.

Quando a opção -c ou --clear é aprovada, então o histórico é apagado.
Quando a opção -l ou --limit é aprovada, ele define o novo limite de histórico. Requer um argumento adicional dizendo a quantas entradas você quer que a história se limite a
Use a opção -ql ou --querylimit para ver o valor limite atual.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="59"/>
      <source>number</source>
      <translation>número</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="66"/>
      <source>Console history erased.</source>
      <translation>Histórico apagado do console.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="75"/>
      <source>Invalid number: %1</source>
      <translation>Número inválido: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="80"/>
      <source>History limit set to %1</source>
      <translation>Limite de histórico definido para %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandMode</name>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="9"/>
      <source>Current results printing mode: %1</source>
      <translation>Modo de impressão atual de resultados: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="16"/>
      <source>Invalid results printing mode: %1</source>
      <translation>Modo de impressão de resultados inválido: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="21"/>
      <source>New results printing mode: %1</source>
      <translation>Modo de impressão de novos resultados: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="26"/>
      <source>tells or changes the query results format</source>
      <translation>chama ou muda o formato dos resultados da consulta</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="31"/>
      <source>When called without argument, tells the current output format for a query results. When the &lt;mode&gt; is passed, the mode is changed to the given one. Supported modes are:
- CLASSIC - columns are separated by a comma, not aligned,
- FIXED   - columns have equal and fixed width, they always fit into terminal window width, but the data in columns can be cut off,
- COLUMNS - like FIXED, but smarter (do not use with huge result sets, see details below),
- ROW     - each column from the row is displayed in new line, so the full data is displayed.

The CLASSIC mode is recommended if you want to see all the data, but you don&apos;t want to waste lines for each column. Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. The CLASSIC mode also doesn&apos;t respect the width of your terminal (console) window, so if values in columns are wider than the window, the row will be continued in next lines.

The FIXED mode is recommended if you want a readable output and you don&apos;t care about long data values. Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window and a number of columns.

The COLUMNS mode is similar to FIXED mode, except it tries to be smart and make columns with shorter values more thin, while columns with longer values get more space. First to shrink are columns with longest headers (so the header names are to be cut off as first), then columns with the longest values are shrinked, up to the moment when all columns fit into terminal window.
ATTENTION! The COLUMNS mode reads all the results from the query at once in order to evaluate column widths, therefore it is dangerous to use this mode when working with huge result sets. Keep in mind that this mode will load entire result set into memory.

The ROW mode is recommended if you need to see whole values and you don&apos;t expect many rows to be displayed, because this mode displays a line of output per each column, so you&apos;ll get 10 lines for single row with 10 columns, then if you have 10 of such rows, you will get 100 lines of output (+1 extra line per each row, to separate rows from each other).</source>
      <translation>Quando chamado sem argumento, informa o formato de saída atual para os resultados de uma consulta. Quando o &lt;mode&gt; é passado, o modo é alterado para o dado. Os modos suportados são:
- CLASSIC - as colunas são separadas por vírgula, não alinhadas,
- FIXED - as colunas têm largura igual e fixa, sempre cabem na largura da janela do terminal, mas os dados nas colunas podem ser cortados,
- COLUMNS - como FIXED, mas mais inteligente (não use com grandes conjuntos de resultados, veja os detalhes abaixo),
- ROW - cada coluna da linha é exibida em nova linha, portanto, os dados completos são exibidos.

O modo CLASSIC é recomendado se você quiser ver todos os dados, mas não quer desperdiçar linhas para cada coluna. Cada linha exibirá dados completos para cada coluna, mas isso também significa que as colunas não serão alinhadas entre si nas próximas linhas. O modo CLASSIC também não respeita a largura da janela do seu terminal (console), portanto, se os valores nas colunas forem mais largos que a janela, a linha será continuada nas próximas linhas.

O modo FIXED é recomendado se você deseja uma saída legível e não se preocupa com valores de dados longos. As colunas serão alinhadas, tornando a saída uma boa tabela. A largura das colunas é calculada a partir da largura da janela do console e um número de colunas.

O modo COLUMNS é semelhante ao modo FIXED, exceto que tenta ser inteligente e tornar as colunas com valores mais curtos mais finas, enquanto as colunas com valores mais longos obtêm mais espaço. As primeiras a encolher são as colunas com cabeçalhos mais longos (portanto, os nomes dos cabeçalhos devem ser cortados primeiro), depois as colunas com os valores mais longos são reduzidas, até o momento em que todas as colunas cabem na janela do terminal.
ATENÇÃO! O modo COLUMNS lê todos os resultados da consulta de uma vez para avaliar as larguras das colunas, portanto, é perigoso usar esse modo ao trabalhar com grandes conjuntos de resultados. Lembre-se de que este modo carregará todo o conjunto de resultados na memória.

O modo ROW é recomendado se você precisa ver valores inteiros e não espera que muitas linhas sejam exibidas, porque este modo exibe uma linha de saída por cada coluna, então você obterá 10 linhas para uma única linha com 10 colunas, então, se você tiver 10 dessas linhas, obterá 100 linhas de saída (+1 linha extra por cada linha, para separar as linhas umas das outras).</translation>
    </message>
  </context>
  <context>
    <name>CliCommandNullValue</name>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
      <source>Current NULL representation string: %1</source>
      <translation>String de denominação NULL atual: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
      <source>tells or changes the NULL representation string</source>
      <translation>chama ou muda a cadeia de representação NULL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
      <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
      <translation>Se nenhum argumento foi aprovado, utilizar a denominação de NULL atual (ou seja, o que é impresso no lugar de valores NULL nos resultados de consultas). Se o argumento é dado, então será usado como uma nova string a ser usada para denominação NULL.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandOpen</name>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="12"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Não é possível chamar %1 quando nenhum banco de dados está definido como atual. Especifique o banco de dados atual com o comando %2 ou passe o nome do banco de dados para %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="29"/>
      <source>Could not add database %1 to list.</source>
      <translation>Não foi possível adicionar o banco de dados %1 à lista.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="37"/>
      <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
      <translation>O arquivo %1 não existe em %2. Não é possível abrir banco de dados inexistente com o comando %3. Para criar um novo banco de dados, use o comando %4.
</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="61"/>
      <source>Database %1 has been open and set as the current working database.</source>
      <translation>Banco de dados %1 foi aberto e definido como a base de dados atual de trabalho.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="66"/>
      <source>opens database connection</source>
      <translation>abre conexão com base de dados</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="71"/>
      <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
      <translation>Abrir conexão com a base de dados. Se nenhum argumento adicional foi aprovado, então a conexão está aberta para o banco de dados padrão atual (veja a ajuda para %1 para detalhes). No entanto, se um argumento foi aprovado, pode ser &lt;name&gt; da base de dados registrada para abrir. ou pode ser &lt;path&gt; para o arquivo de banco de dados para abrir. No segundo caso, o &lt;path&gt; é registrado na lista com um nome gerado, mas apenas para o período da sessão de aplicação atual. Depois de reiniciar o aplicativo, esse banco de dados não será restaurado na lista.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>caminho</translation>
    </message>
  </context>
  <context>
    <name>CliCommandPwd</name>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="13"/>
      <source>prints the current working directory</source>
      <translation>mostra o diretório de trabalho atual</translation>
    </message>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="18"/>
      <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
      <translation>Isso é o mesmo que &apos;pwd&apos; comando em sistemas Unix e &apos;cd&apos; comando sem argumentos no Windows. Imprime o diretório de trabalho atual. Você pode alterar o diretório de trabalho atual com %1 comando e você também pode listar o conteúdo do diretório de trabalho atual com %2 comando.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandRemove</name>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="12"/>
      <source>No such database: %1</source>
      <translation>Banco de dados não existe: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="20"/>
      <source>Database removed: %1</source>
      <translation>Banco de dados removido: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="26"/>
      <source>New current database set:</source>
      <translation>Novo banco de dados definido:</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="35"/>
      <source>removes database from the list</source>
      <translation>remove o banco de dados da lista</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="40"/>
      <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
      <translation>Remove &lt;name&gt; do banco de dados da lista de bancos de dados registrados. Se o banco de dados não estava na lista (ver %1 comando), então a mensagem de erro é impressa e nada mais acontece.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
  </context>
  <context>
    <name>CliCommandSql</name>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="19"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Nenhum banco de dados está definido.
Execute %1 para definir o banco de dados ativo.
Execute %2 para ver a lista de todos os bancos de dados.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="30"/>
      <source>Database is not open.</source>
      <translation>Banco de dados não está aberto.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="65"/>
      <source>executes SQL query</source>
      <translation>executa consulta SQL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="70"/>
      <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
      <translation>Este comando é executado toda vez que você digitar a consulta SQL no prompt de comando. Executa a consulta no banco de dados de trabalho atual (veja ajuda para %1 para detalhes). Não faz sentido executar este comando explicitamente. Em vez disso, digite a consulta SQL no prompt de comando, sem qualquer comando prefixado.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="86"/>
      <source>sql</source>
      <comment>CLI command syntax</comment>
      <translation>sql</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="135"/>
      <location filename="../commands/clicommandsql.cpp" line="177"/>
      <source>Too many columns to display in %1 mode.</source>
      <translation>Muitas colunas para serem exibidas no modo %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="254"/>
      <source>Row %1</source>
      <translation>Linha %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="404"/>
      <source>Query execution error: %1</source>
      <translation>Erro na execução da consulta: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTables</name>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="15"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Nenhum banco de dados: %1. Use %2 para ver a lista de bancos de dados existentes.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="25"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Não é possível chamar %1 quando nenhum banco de dados está definido como atual. Especifique o banco de dados atual com o comando %2 ou passe o nome do banco de dados para %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="32"/>
      <source>Database %1 is closed.</source>
      <translation>Banco de dados %1 está fechado.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="45"/>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Database</source>
      <translation>Banco de dados</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Table</source>
      <translation>Tabela</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="61"/>
      <source>prints list of tables in the database</source>
      <translation>lista as tabelas do banco de dados</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="66"/>
      <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
      <translation>Mostra a lista das tabelas de acordo com &lt;database&gt; ou no atual banco de dados de trabalho. Note que o &lt;database&gt; deve ser o nome do banco de dados registrado (ver %1). A lista de saída inclui todas as tabelas de qualquer outro banco de dados anexado à base de dados solicitada.
Quando a opção -s é dada, então as tabelas do sistema também são listadas.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="77"/>
      <source>database</source>
      <comment>CLI command syntax</comment>
      <translation>banco de dados</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTree</name>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="12"/>
      <source>No current working database is selected. Use %1 to define one and then run %2.</source>
      <translation>Nenhum banco de dados selecionado. Use %1 para definir um banco de dados e depois execute %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="54"/>
      <source>Tables</source>
      <translation>Tabelas</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="58"/>
      <source>Views</source>
      <translation>Visualizações</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="83"/>
      <source>Columns</source>
      <translation>Colunas</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="88"/>
      <source>Indexes</source>
      <translation>Índices</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="92"/>
      <location filename="../commands/clicommandtree.cpp" line="113"/>
      <source>Triggers</source>
      <translation>Triggers</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="132"/>
      <source>prints all objects in the database as a tree</source>
      <translation>mostra todos os objetos no banco do dados como uma árvore</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="137"/>
      <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
      <translation>Imprime todos os objetos (tabelas, indexes, gatilhos e visualizações) que estão no banco de dados como uma árvore. A árvore é muito parecida com a que você pode ver no cliente GUI do SQLiteStudio.
Quando a opção -c é dada, então as colunas também serão listadas sob cada tabela.
Quando a opção -s é dada, então também objetos do sistema serão impressos (sqlite_* tabelas, índices de auto-incremento, etc).
O argumento do banco de dados é opcional e, se fornecido, apenas o banco de dados informado será impresso. Este não é um nome de banco de dados registrado, mas em vez disso é um nome de banco de dados SQLite interno, como &apos;main&apos;, &apos;temp&apos;, ou qualquer nome de banco de dados anexado. Para imprimir árvore para outro banco de dados registrado, chame %1 primeiro para mudar o banco de dados de trabalho e, em seguida, use o comando %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandUse</name>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="13"/>
      <source>No current database selected.</source>
      <translation>Nenhum banco de dados selecionado.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="16"/>
      <location filename="../commands/clicommanduse.cpp" line="30"/>
      <source>Current database: %1</source>
      <translation>Banco de dados atual: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="23"/>
      <source>No such database: %1</source>
      <translation>Banco de dados não existe: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="35"/>
      <source>changes default working database</source>
      <translation>definir o banco de dados padrão</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="40"/>
      <source>Changes current working database to &lt;name&gt;. If the &lt;name&gt; database is not registered in the application, then the error message is printed and no change is made.

What is current working database?
When you type a SQL query to be executed, it is executed on the default database, which is also known as the current working database. Most of database-related commands can also work using default database, if no database was provided in their arguments. The current database is always identified by command line prompt. The default database is always defined (unless there is no database on the list at all).

The default database can be selected in various ways:
- using %1 command,
- by passing database file name to the application startup parameters,
- by passing registered database name to the application startup parameters,
- by restoring previously selected default database from saved configuration,
- or when default database was not selected by any of the above, then first database from the registered databases list becomes the default one.</source>
      <translation>Altera a base de dados atual de trabalho para &lt;name&gt;. Se o banco de dados &lt;name&gt; não estiver registrado, então a mensagem de erro é mostrada e nenhuma alteração é feita.

O que é uma base de dados em funcionamento?
Quando você digita uma consulta SQL a ser executada, ela é executada no banco de dados padrão, que é também conhecida como a banco de dados de trabalho atual. A maioria dos comandos relacionados ao banco de dados também pode funcionar usando o banco de dados padrão, se nenhum banco de dados foi fornecido em seus argumentos. A base de dados atual é sempre identificada pela linha de comando. O banco de dados padrão é sempre definido (a menos que não haja nenhum banco de dados na lista).

O banco de dados padrão pode ser selecionado de várias maneiras:
- usando o comando %1
- passando o nome do arquivo de banco de dados para o aplicativo parâmetros de inicialização,
- passando o nome do banco de dados registrado para os parâmetros de inicialização do aplicativo,
- restaurando o banco de dados padrão selecionado anteriormente a partir da configuração salva,
- ou quando o banco de dados padrão não foi selecionado por nenhum dos itens acima então primeiro banco de dados da lista de bancos de dados registrados torna-se o padrão.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="63"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../clicommandsyntax.cpp" line="155"/>
      <source>Insufficient number of arguments.</source>
      <translation>Número insuficiente de argumentos.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="325"/>
      <source>Too many arguments.</source>
      <translation>Poucos argumentos.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="347"/>
      <source>Invalid argument value: %1.
Expected one of: %2</source>
      <translation>Valor do argumento inválido: %1.
Espera-se um de: %2</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="383"/>
      <source>Unknown option: %1</source>
      <comment>CLI command syntax</comment>
      <translation>Opção desconhecida: %1</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="394"/>
      <source>Option %1 requires an argument.</source>
      <comment>CLI command syntax</comment>
      <translation>A opção %1 requer um argumento.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
      <source>string</source>
      <comment>CLI command syntax</comment>
      <translation>string</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="28"/>
      <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
      <translation>Interface da linha de comando para SQLiteStudio, um gerenciador para SQLite.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="32"/>
      <source>Enables debug messages on standard error output.</source>
      <translation>Habilita mensagens de depuração padrão na saída de erro.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="33"/>
      <source>Enables Lemon parser debug messages for SQL code assistant.</source>
      <translation>Habilita mensagens de depuração do analisador Lemon no assistente de código SQL.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="34"/>
      <source>Lists plugins installed in the SQLiteStudio and quits.</source>
      <translation>Lista os plugins instalados no SQLiteStudio e encerrados.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="36"/>
      <source>Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</source>
      <translation>Executa o arquivo SQL fornecido (incluindo todos os recursos avançados do executor de consulta SQLiteStudio &apos;s) no arquivo de banco de dados especificado e fecha a rotina. O parâmetro do banco de dados torna-se obrigatório se esta opção for utilizada.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="39"/>
      <source>SQL file</source>
      <translation>Arquivo SQL</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="40"/>
      <source>Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</source>
      <translation>Codificação de caracteres utilizada ao ler o arquivo SQL (-e option). Use -cl para listar os codecs disponíveis. O padrão é %1.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="43"/>
      <source>codec</source>
      <translation>Codec</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="44"/>
      <source>Lists available codecs to be used with -c option and quits.</source>
      <translation>Lista de codecs disponíveis para serem usados com opção -c e encerramento.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="46"/>
      <source>When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</source>
      <translation>Quando usado em conjunto com a opção -e, a execução não parará em um erro, mas sim continuará até o fim, ignorando erros.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>file</source>
      <translation>arquivo</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>Database file to open</source>
      <translation>Arquivo do banco de dados para abrir</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="78"/>
      <source>Invalid codec: %1. Use -cl option to list available codecs.</source>
      <translation>Codec inválido: %1. Use a opção -cl para listar codecs disponíveis.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="108"/>
      <source>Database file argument is mandatory when executing SQL file.</source>
      <translation>O argumento do arquivo de banco de dados é obrigatório para executar arquivo SQL.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="114"/>
      <source>Could not open specified database for executing SQL file. You may try using -d option to find out more details.</source>
      <translation type="unfinished">Could not open specified database for executing SQL file. You may try using -d option to find out more details.</translation>
    </message>
  </context>
</TS>
