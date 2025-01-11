<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pt-BR" sourcelanguage="en">
  <context>
    <name>AbstractDb</name>
    <message>
      <location filename="../db/abstractdb.cpp" line="351"/>
      <location filename="../db/abstractdb.cpp" line="368"/>
      <source>Cannot execute query on closed database.</source>
      <translation>Não é possível executar a consulta com o banco de dados fechado.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="708"/>
      <source>Error attaching database %1: %2</source>
      <translation>Erro ao anexar banco de dados %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="957"/>
      <source>Failed to make full WAL checkpoint on database &apos;%1&apos;. Error returned from SQLite engine: %2</source>
      <translation>Falha ao fazer checkpoint WAL cheio no banco de dados &apos;%1&apos;. Erro retornado do mecanismo SQLite: %2</translation>
    </message>
  </context>
  <context>
    <name>ChainExecutor</name>
    <message>
      <location filename="../db/chainexecutor.cpp" line="37"/>
      <source>The database for executing queries was not defined.</source>
      <comment>chain executor</comment>
      <translation>Não foi especificado banco de dados para execução das consultas. </translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="44"/>
      <source>The database for executing queries was not open.</source>
      <comment>chain executor</comment>
      <translation>O banco de dados para execução das consultas não foi aberto.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="54"/>
      <source>Could not disable foreign keys in the database. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Não foi possível desativar as chaves estrangeiras do banco de dados. Detalhes: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="62"/>
      <source>Could not start a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Não foi possível iniciar a transação do banco de dados. Detalhes: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="89"/>
      <source>Interrupted</source>
      <comment>chain executor</comment>
      <translation>Interrompido</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="151"/>
      <source>Could not commit a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Não foi possível efetuar commit no banco de dados. Detalhes: %1</translation>
    </message>
  </context>
  <context>
    <name>CompletionHelper</name>
    <message>
      <location filename="../completionhelper.cpp" line="158"/>
      <source>New row reference</source>
      <translation>Nova referência de linha</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="165"/>
      <source>Old row reference</source>
      <translation>Referência de linha antiga</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="170"/>
      <source>New table name</source>
      <translation>Nome da nova tabela</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="173"/>
      <source>New index name</source>
      <translation>Nome do novo índice</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="176"/>
      <source>New view name</source>
      <translation>Nome da nova visualização</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="179"/>
      <source>New trigger name</source>
      <translation>Nome da nova trigger</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="182"/>
      <source>Table or column alias</source>
      <translation>Apelido da tabela ou coluna</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="185"/>
      <source>transaction name</source>
      <translation>nome da transação</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="188"/>
      <source>New column name</source>
      <translation>Nome da nova coluna</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="191"/>
      <source>Column data type</source>
      <translation>Tipo de dados de coluna</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="194"/>
      <source>Constraint name</source>
      <translation>Nome da constraint</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="207"/>
      <source>Error message</source>
      <translation>Mensagem de Erro</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="256"/>
      <source>Any word</source>
      <translation>Qualquer palavra</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="259"/>
      <source>String</source>
      <translation type="unfinished">String</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="262"/>
      <location filename="../completionhelper.cpp" line="265"/>
      <source>Number</source>
      <translation type="unfinished">Number</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="277"/>
      <source>BLOB literal</source>
      <translation type="unfinished">BLOB literal</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="437"/>
      <source>Default database</source>
      <translation>Banco de dados padrão</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="438"/>
      <source>Temporary objects database</source>
      <translation>Banco de dados de objetos temporários</translation>
    </message>
  </context>
  <context>
    <name>ConfigImpl</name>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="881"/>
      <source>Could not start database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Não foi possível iniciar a transação do banco de dados para excluir o histórico do SQL. Portanto, ela não será excluída.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="888"/>
      <source>Could not commit database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Não foi possível submeter a transação do banco de dados para excluir o histórico do SQL. Portanto, não será excluída.</translation>
    </message>
  </context>
  <context>
    <name>DbManagerImpl</name>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="64"/>
      <source>Could not add database %1: %2</source>
      <translation>Não foi possível adicionar o banco de dados %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="137"/>
      <source>Database %1 could not be updated, because of an error: %2</source>
      <translation>Banco de dados %1 não pode ser atualizado, devido a um erro: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="365"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="394"/>
      <source>Database file doesn&apos;t exist.</source>
      <translation>Arquivo de banco de dados não existe.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="367"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="396"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="615"/>
      <source>No supporting plugin loaded.</source>
      <translation>Nenhum plugin de suporte carregado.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="533"/>
      <source>Database could not be initialized.</source>
      <translation>Banco de dados não pode ser inicializado.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="543"/>
      <source>No suitable database driver plugin found.</source>
      <translation>Nenhum plugin de driver de base de dados adequado encontrado.</translation>
    </message>
  </context>
  <context>
    <name>DbObjectOrganizer</name>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="366"/>
      <location filename="../dbobjectorganizer.cpp" line="397"/>
      <source>Error while creating table in target database: %1</source>
      <translation>Erro ao criar tabela no banco de dados: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="366"/>
      <source>Could not parse table.</source>
      <translation>Não foi possível analisar a tabela.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="411"/>
      <source>Database %1 could not be attached to database %2, so the data of table %3 will be copied with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.</source>
      <translation>Banco de dados %1 não pôde ser ligado ao banco de dados %2, de modo que os dados da tabela %3 vão ser copiados com o SQLiteStudio como um mediador. Este método pode ser lento para grande tabelas, por favor seja paciente. </translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="435"/>
      <source>Error while copying data for table %1: %2</source>
      <translation>Erro ao copiar dados da tabela %1:%2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="454"/>
      <location filename="../dbobjectorganizer.cpp" line="461"/>
      <location filename="../dbobjectorganizer.cpp" line="488"/>
      <source>Error while copying data to table %1: %2</source>
      <translation>Erro ao copiar dados para a tabela %1:%2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="510"/>
      <source>Error while dropping source view %1: %2
Tables, indexes, triggers and views copied to database %3 will remain.</source>
      <translation>Erro ao soltar a visão da fonte %1: %2
Tabelas, índices, trigger e visualizações copiadas para o banco de dados %3 permanecerão.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="517"/>
      <source>Error while creating view in target database: %1</source>
      <translation>Erro ao criar a visualização no banco de dados destino: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="522"/>
      <source>Error while creating index in target database: %1</source>
      <translation>Erro ao criar o índice no banco de dados destino: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="527"/>
      <source>Error while creating trigger in target database: %1</source>
      <translation>Erro ao criar gatilho no banco de dados de destino: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="667"/>
      <location filename="../dbobjectorganizer.cpp" line="674"/>
      <location filename="../dbobjectorganizer.cpp" line="683"/>
      <source>Could not parse object &apos;%1&apos; in order to move or copy it.</source>
      <translation>Não foi possível analisar o objeto &apos;%1&apos; para mover ou copiar.</translation>
    </message>
  </context>
  <context>
    <name>DdlHistoryModel</name>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="66"/>
      <source>Database name</source>
      <comment>ddl history header</comment>
      <translation>Nome do banco de dados</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="68"/>
      <source>Database file</source>
      <comment>ddl history header</comment>
      <translation>Arquivo de banco de dados</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="70"/>
      <source>Date of execution</source>
      <comment>ddl history header</comment>
      <translation>Data de execução</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="72"/>
      <source>Changes</source>
      <comment>ddl history header</comment>
      <translation>Alterações</translation>
    </message>
  </context>
  <context>
    <name>ExportManager</name>
    <message>
      <location filename="../services/exportmanager.cpp" line="71"/>
      <source>Export plugin %1 doesn&apos;t support exporing query results.</source>
      <translation>Plugin de exportação %1 não suporta resultados de consulta suprimindo.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="97"/>
      <source>Export plugin %1 doesn&apos;t support exporing tables.</source>
      <translation>O plugin %1 de exportação não suporta&apos;tabelas de exportação.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="121"/>
      <source>Export plugin %1 doesn&apos;t support exporing databases.</source>
      <translation>Plugin de exportação %1 não suporta resultados de consulta suprimindo.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="154"/>
      <source>Export format &apos;%1&apos; is not supported. Supported formats are: %2.</source>
      <translation>Formato de exportação &apos;%1&apos; não é suportado. Formatos suportados são: %2.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="218"/>
      <source>Export to the clipboard was successful.</source>
      <translation>Exportação para área de transferência com sucesso.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="222"/>
      <source>Export to the file &apos;%1&apos; was successful.</source>
      <translation>Exportação para o arquivo &apos;%1&apos; foi bem sucedida.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="224"/>
      <source>Export was successful.</source>
      <translation>Exportação bem-sucedida.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="266"/>
      <source>Could not export to file %1. File cannot be open for writting.</source>
      <translation>Não foi possível exportar para o arquivo %1. O arquivo não pode ser aberto para escrita.</translation>
    </message>
  </context>
  <context>
    <name>ExportWorker</name>
    <message>
      <location filename="../exportworker.cpp" line="122"/>
      <source>Error while exporting query results: %1</source>
      <translation>Erro ao exportar os resultados da consulta: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="208"/>
      <source>Error while counting data column width to export from query results: %1</source>
      <translation>Erro ao contar a largura da coluna de dados para exportar dos resultados da consulta: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="352"/>
      <location filename="../exportworker.cpp" line="410"/>
      <source>Could not parse %1 in order to export it. It will be excluded from the export output.</source>
      <translation>Não foi possível analisar %1 para exportá-lo. Ele será excluído da saída de exportação.</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="614"/>
      <source>Error while reading data to export from table %1: %2</source>
      <translation>Erro ao ler os dados para exportar da tabela %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="622"/>
      <source>Error while counting data to export from table %1: %2</source>
      <translation>Erro ao contar os dados a serem exportados da tabela %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="638"/>
      <source>Error while counting data column width to export from table %1: %2</source>
      <translation>Erro ao contar a largura da coluna de dados para exportar da tabela %1: %2</translation>
    </message>
  </context>
  <context>
    <name>FunctionManagerImpl</name>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="199"/>
      <source>Could not create scripting context, probably the plugin is not configured properly</source>
      <translation type="unfinished">Could not create scripting context, probably the plugin is not configured properly</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="292"/>
      <source>Invalid number of arguments to function &apos;%1&apos;. Expected %2, but got %3.</source>
      <translation>Número inválido de argumentos para a função &apos;%1&apos;. Esperado %2, mas tem %3.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="406"/>
      <source>No such function registered in SQLiteStudio: %1(%2)</source>
      <translation>Nenhuma função registrada no SQLiteStudio: %1(%2)</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="412"/>
      <source>Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.</source>
      <translation>A função %1(%2) foi registrada no idioma %3, mas o plugin que suporta essa linguagem não está atualmente carregado.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="430"/>
      <source>Invalid regular expression pattern: %1</source>
      <translation>Expressão regular inválida: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="449"/>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="482"/>
      <source>Could not open file %1 for reading: %2</source>
      <translation>Não foi possível abrir o arquivo %1 para leitura: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="504"/>
      <source>Could not open file %1 for writting: %2</source>
      <translation>Não foi possível abrir o arquivo %1 para escrita: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="524"/>
      <source>Error while writting to file %1: %2</source>
      <translation>Erro ao gravar o arquivo %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="542"/>
      <source>Unsupported scripting language: %1</source>
      <translation>Idioma do script não suportado: %1</translation>
    </message>
  </context>
  <context>
    <name>GenericExportPlugin</name>
    <message>
      <location filename="../plugins/genericexportplugin.cpp" line="20"/>
      <source>Could not initialize text codec for exporting. Using default codec: %1</source>
      <translation>Não foi possível inicializar o codec de texto para exportação. Usando o codec padrão: %1</translation>
    </message>
  </context>
  <context>
    <name>ImportManager</name>
    <message>
      <location filename="../services/importmanager.cpp" line="99"/>
      <source>Imported data to the table &apos;%1&apos; successfully. Number of imported rows: %2</source>
      <translation>Dados importados para a tabela &apos;%1&apos; com sucesso. Número de linhas importadas: %2</translation>
    </message>
  </context>
  <context>
    <name>ImportWorker</name>
    <message>
      <location filename="../importworker.cpp" line="24"/>
      <source>No columns provided by the import plugin.</source>
      <translation>Nenhuma coluna fornecida pelo plugin de importação.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="31"/>
      <source>Could not start transaction in order to import a data: %1</source>
      <translation>Não foi possível iniciar a transação para importar os dados: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="54"/>
      <source>Could not commit transaction for imported data: %1</source>
      <translation>Não foi possível submeter a transação para dados importados: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="101"/>
      <source>Table &apos;%1&apos; has less columns than there are columns in the data to be imported. Excessive data columns will be ignored.</source>
      <translation>Tabela &apos;%1&apos; tem menos colunas que há nos dados a serem importados. Colunas de dados excessivas serão ignoradas.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="106"/>
      <source>Table &apos;%1&apos; has more columns than there are columns in the data to be imported. Some columns in the table will be left empty.</source>
      <translation>Tabela &apos;%1&apos; tem mais colunas que há colunas nos dados a serem importados. Algumas colunas na tabela serão deixadas vazias.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="125"/>
      <source>Could not create table to import to: %1</source>
      <translation>Não foi possível criar a tabela para importar para: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="134"/>
      <location filename="../importworker.cpp" line="185"/>
      <location filename="../importworker.cpp" line="192"/>
      <source>Error while importing data: %1</source>
      <translation>Erro ao importar dados: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="134"/>
      <location filename="../importworker.cpp" line="192"/>
      <source>Interrupted.</source>
      <comment>import process status update</comment>
      <translation>Interrompido.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="180"/>
      <source>Could not import data row number %1. The row was ignored. Problem details: %2</source>
      <translation>Não foi possível importar a linha de dados número %1. A linha foi ignorada. Detalhes do problema: %2</translation>
    </message>
  </context>
  <context>
    <name>PluginManagerImpl</name>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="546"/>
      <source>Cannot load plugin %1, because it&apos;s in conflict with plugin %2.</source>
      <translation>Não foi possível carregar o plugin %1, porque está em conflito com o plugin %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="557"/>
      <source>Cannot load plugin %1, because its dependency was not loaded: %2.</source>
      <translation>Não foi possível carregar o plugin %1, porque sua dependência não foi carregada: %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="566"/>
      <source>Cannot load plugin %1. Error details: %2</source>
      <translation>Não foi possível carregar o plugin %1. Detalhes do erro: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="582"/>
      <source>Cannot load plugin %1 (error while initializing plugin).</source>
      <translation>Não é possível carregar o plugin %1 (erro durante a inicialização do plugin).</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="743"/>
      <source>min: %1</source>
      <comment>plugin dependency version</comment>
      <translation>min: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="744"/>
      <source>max: %1</source>
      <comment>plugin dependency version</comment>
      <translation>max: %1</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstant</name>
    <message>
      <location filename="../plugins/populateconstant.cpp" line="10"/>
      <source>Constant</source>
      <comment>populate constant plugin name</comment>
      <translation>Constante</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstantConfig</name>
    <message>
      <location filename="../plugins/populateconstant.ui" line="20"/>
      <source>Constant value:</source>
      <translation>Valor constante:</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionary</name>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="17"/>
      <source>Dictionary</source>
      <comment>dictionary populating plugin name</comment>
      <translation>Dicionário</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionaryConfig</name>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="20"/>
      <source>Dictionary file</source>
      <translation>Arquivo de dicionário</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="29"/>
      <source>Pick dictionary file</source>
      <translation>Escolher arquivo do dicionário</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="39"/>
      <source>Word separator</source>
      <translation>Separador de palavras</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="45"/>
      <source>Whitespace</source>
      <translation>Espaço em branco</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="58"/>
      <source>Line break</source>
      <translation>Quebra de linha</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="74"/>
      <source>Method of using words</source>
      <translation>Método de uso palavras</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="80"/>
      <source>Ordered</source>
      <translation>Ordenado</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="93"/>
      <source>Randomly</source>
      <translation>Aleatoriamente</translation>
    </message>
  </context>
  <context>
    <name>PopulateManager</name>
    <message>
      <location filename="../services/populatemanager.cpp" line="89"/>
      <source>Table &apos;%1&apos; populated successfully.</source>
      <translation>Tabela &apos;%1&apos; preenchida com sucesso.</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandom</name>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="13"/>
      <source>Random number</source>
      <translation>Número aleatório</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomConfig</name>
    <message>
      <location filename="../plugins/populaterandom.ui" line="20"/>
      <source>Constant prefix</source>
      <translation>Prefixo da constante</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="26"/>
      <source>No prefix</source>
      <translation>Nenhum prefixo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="39"/>
      <source>Minimum value</source>
      <translation>Valor mínimo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="61"/>
      <source>Maximum value</source>
      <translation>Valor máximo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="86"/>
      <source>Constant suffix</source>
      <translation>Sufixo da constante</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="92"/>
      <source>No suffix</source>
      <translation>Nenhum sufixo</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomText</name>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="14"/>
      <source>Random text</source>
      <translation>Texto aleatório</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomTextConfig</name>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="20"/>
      <source>Use characters from common sets:</source>
      <translation>Usar caracteres de conjuntos comuns:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="36"/>
      <source>Minimum length</source>
      <translation>Comprimento mínimo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="64"/>
      <source>Letters from a to z.</source>
      <translation>Letras de a a z.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="67"/>
      <source>Alpha</source>
      <translation>Alfa</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="77"/>
      <source>Numbers from 0 to 9.</source>
      <translation>Números de 0 a 9.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="80"/>
      <source>Numeric</source>
      <translation>Numérico</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="90"/>
      <source>A whitespace, a tab and a new line character.</source>
      <translation>Um espaço em branco, uma aba e um novo caractere de linha.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="93"/>
      <source>Whitespace</source>
      <translation>Espaço em branco</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="103"/>
      <source>Includes all above and all others.</source>
      <translation>Inclui todos acima e todos os outros.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="106"/>
      <source>Binary</source>
      <translation>Binário</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="119"/>
      <source>Use characters from my custom set:</source>
      <translation>Usar caracteres do meu grupo personalizado:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="132"/>
      <source>Maximum length</source>
      <translation>Comprimento máximo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="160"/>
      <source>If you type some character multiple times, it&apos;s more likely to be used.</source>
      <translation>Se você digitar um caractere várias vezes, é mais provável que seja usado.</translation>
    </message>
  </context>
  <context>
    <name>PopulateScript</name>
    <message>
      <location filename="../plugins/populatescript.cpp" line="34"/>
      <source>Script</source>
      <translation>Script</translation>
    </message>
  </context>
  <context>
    <name>PopulateScriptConfig</name>
    <message>
      <location filename="../plugins/populatescript.ui" line="26"/>
      <source>Initialization code (optional)</source>
      <translation>Código de inicialização (opcional)</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="45"/>
      <source>Per step code</source>
      <translation>Código detalhado</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="70"/>
      <source>Language</source>
      <translation>Idioma</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="89"/>
      <source>Help</source>
      <translation>Ajuda</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequence</name>
    <message>
      <location filename="../plugins/populatesequence.cpp" line="13"/>
      <source>Sequence</source>
      <translation>Sequência</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequenceConfig</name>
    <message>
      <location filename="../plugins/populatesequence.ui" line="33"/>
      <source>Start value:</source>
      <translation>Valor inicial:</translation>
    </message>
    <message>
      <location filename="../plugins/populatesequence.ui" line="56"/>
      <source>Step:</source>
      <translation>Etapas:</translation>
    </message>
  </context>
  <context>
    <name>PopulateWorker</name>
    <message>
      <location filename="../populateworker.cpp" line="23"/>
      <source>Could not start transaction in order to perform table populating. Error details: %1</source>
      <translation>Não foi possível iniciar a transação para realizar o preenchimento da tabela. Detalhes do erro: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="69"/>
      <source>Error while populating table: %1</source>
      <translation>Erro ao preencher a tabela: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="80"/>
      <source>Could not commit transaction after table populating. Error details: %1</source>
      <translation>Não foi possível submeter a transação após o preenchimento da tabela. Detalhes de erro: %1</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../common/utils.cpp" line="939"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Não foi possível abrir o arquivo &apos;%1&apos; para leitura: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="437"/>
      <source>Could not open database: %1</source>
      <translation>Não foi possível abrir o banco de dados: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="1235"/>
      <source>Result set expired or no row available.</source>
      <translation>Conjunto de resultados expirado ou nenhuma linha disponível.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="333"/>
      <location filename="../db/abstractdb3.h" line="337"/>
      <source>Could not load extension %1: %2</source>
      <translation>Não foi possível carregar a extensão %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="423"/>
      <source>Could not run WAL checkpoint: %1</source>
      <translation>Não foi possível executar o ponto de verificação do WAL: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="461"/>
      <source>Could not close database: %1</source>
      <translation>Não foi possível fechar o banco de dados: %1</translation>
    </message>
    <message>
      <location filename="../impl/dbattacherimpl.cpp" line="114"/>
      <source>Could not attach database %1: %2</source>
      <translation>Não foi possível anexar o banco de dados %1: %2</translation>
    </message>
    <message>
      <location filename="../parser/parsercontext.cpp" line="108"/>
      <location filename="../parser/parsercontext.cpp" line="110"/>
      <source>Incomplete query.</source>
      <translation>Consulta incompleta.</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="2519"/>
      <source>Parser stack overflow</source>
      <translation>Parser stack overflow (estourado)</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="5954"/>
      <source>Syntax error</source>
      <translation>Erro de sintaxe</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="32"/>
      <source>Could not open dictionary file %1 for reading.</source>
      <translation>Não foi possível abrir o arquivo de dicionário %1 para leitura.</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="93"/>
      <source>Dictionary file must exist and be readable.</source>
      <translation>Arquivo de dicionário deve existir e estar legível.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="54"/>
      <source>Maximum value cannot be less than minimum value.</source>
      <translation>O valor máximo não pode ser menor que o valor mínimo.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="79"/>
      <source>Maximum length cannot be less than minimum length.</source>
      <translation>O comprimento máximo não pode ser inferior ao comprimento mínimo.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="90"/>
      <source>Custom character set cannot be empty.</source>
      <translation>O conjunto de caracteres personalizado não pode estar vazio.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="61"/>
      <source>Could not find plugin to support scripting language: %1</source>
      <translation>Não foi possível encontrar o plugin para suportar o idioma do script: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="70"/>
      <source>Could not get evaluation context, probably the %1 scripting plugin is not configured properly</source>
      <translation type="unfinished">Could not get evaluation context, probably the %1 scripting plugin is not configured properly</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="84"/>
      <source>Error while executing populating initial code: %1</source>
      <translation>Erro ao executar o preenchimento do código inicial: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="106"/>
      <source>Error while executing populating code: %1</source>
      <translation>Erro ao executar o código de execução: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="138"/>
      <source>Select implementation language.</source>
      <translation>Selecionar idioma de implementação.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="139"/>
      <source>Implementation code cannot be empty.</source>
      <translation>Código de implementação não pode ser vazio.</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="372"/>
      <source>Could not resolve data source for column: %1</source>
      <translation>Não foi possível resolver a fonte de dados para a coluna: %1</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="444"/>
      <source>Could not resolve table for column &apos;%1&apos;.</source>
      <translation>Não foi possível resolver a tabela para a coluna &apos;%1&apos;.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="761"/>
      <source>Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart. Unable to create a file at following locations: %1.</source>
      <translation>Não foi possível inicializar o arquivo de configuração. Quaisquer alterações de configuração e histórico de consultas serão perdidos após a reinicialização do aplicativo. Não foi possível criar um arquivo nos seguintes locais: %1.</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="347"/>
      <source>General purpose</source>
      <comment>plugin category name</comment>
      <translation>Objetivo geral</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="348"/>
      <source>Database support</source>
      <comment>plugin category name</comment>
      <translation>Suporte do banco de dados</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="349"/>
      <source>Code formatter</source>
      <comment>plugin category name</comment>
      <translation>Formatador de código</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="350"/>
      <source>Scripting languages</source>
      <comment>plugin category name</comment>
      <translation>Linguagens dos scripts</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="352"/>
      <source>Exporting</source>
      <comment>plugin category name</comment>
      <translation>Exportando</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="353"/>
      <source>Importing</source>
      <comment>plugin category name</comment>
      <translation>Importando</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="354"/>
      <source>Table populating</source>
      <comment>plugin category name</comment>
      <translation>Preencher a tabela</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="161"/>
      <source>Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition due to problems while parsing DDL of the table %3.</source>
      <translation>Tabela %1 é tabela de referência %2, mas a definição de chave estrangeira não será atualizada para uma nova definição de tabela devido a problemas ao analisar DDL da tabela %3.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="510"/>
      <source>All columns indexed by the index %1 are gone. The index will not be recreated after table modification.</source>
      <translation>Todas as colunas indexadas pelo índice %1 desapareceram. O índice não será recriado após a modificação da tabela.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="554"/>
      <source>There is problem with proper processing trigger %1. It may be not fully updated afterwards and will need your attention.</source>
      <translation>Há um problema com a trigger %1. Ela pode não ser totalmente atualizada e precisará de sua atenção.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="569"/>
      <source>All columns covered by the trigger %1 are gone. The trigger will not be recreated after table modification.</source>
      <translation>Todas as colunas cobertas pela trigger %1 desapareceram. A trigger não será recriada após a modificação da tabela.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="601"/>
      <source>Cannot not update trigger %1 according to table %2 modification.</source>
      <translation>Não é possível atualizar trigger %1 de acordo com modificação da tabela %2.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="620"/>
      <source>Cannot not update view %1 according to table %2 modifications.
The view will remain as it is.</source>
      <translation>Não é possível atualizar a exibição %1 de acordo com as modificações da tabela %2 .
A visualização permanecerá como é.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="782"/>
      <location filename="../tablemodifier.cpp" line="806"/>
      <location filename="../tablemodifier.cpp" line="825"/>
      <source>There is a problem with updating an %1 statement within %2 trigger. One of the %1 substatements which might be referring to table %3 cannot be properly modified. Manual update of the trigger may be necessary.</source>
      <translation>Ocorreu um problema ao atualizar uma instrução %1 dentro do gatilho %2 . Uma das %1 substâncias que poderiam referir-se à tabela %3 não pode ser devidamente modificada. A atualização manual da trigger pode ser necessária.</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="24"/>
      <source>Could not parse DDL of the view to be created. Details: %1</source>
      <translation>Não foi possível analisar DDL da view a ser criada. Detalhes: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="33"/>
      <source>Parsed query is not CREATE VIEW. It&apos;s: %1</source>
      <translation>A consulta analisada não é CREATE VIEW. É: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="81"/>
      <source>SQLiteStudio was unable to resolve columns returned by the new view, therefore it won&apos;t be able to tell which triggers might fail during the recreation process.</source>
      <translation>O SQLiteStudio não conseguiu resolver colunas retornadas pela nova visualização Portanto, ele não  é capaz de dizer quais trigger podem falhar durante o processo de recriação.</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutor</name>
    <message>
      <location filename="../db/queryexecutor.cpp" line="204"/>
      <source>Execution interrupted.</source>
      <translation>Execução interrompida.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="245"/>
      <source>Database is not open.</source>
      <translation>Banco de dados não está aberto.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="253"/>
      <source>Only one query can be executed simultaneously.</source>
      <translation>Apenas uma consulta pode ser executada simultaneamente.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="343"/>
      <location filename="../db/queryexecutor.cpp" line="357"/>
      <location filename="../db/queryexecutor.cpp" line="607"/>
      <source>An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1</source>
      <translation>Ocorreu um erro ao executar a função count(*), desta forma a paginação de dados será desabilitada. Detalhes de erro do banco de dados: %1</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="526"/>
      <source>SQLiteStudio was unable to extract metadata from the query. Results won&apos;t be editable.</source>
      <translation>SQLiteStudio não pôde extrair os metadados da consulta. Os resultados obtidos não serão editáveis.</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutorSmartHints</name>
    <message>
      <location filename="../db/queryexecutorsteps/queryexecutorsmarthints.cpp" line="77"/>
      <source>Column %1 in table %2 is referencing column %3 in table %4, but these columns have different data types: %5 vs. %6. This may cause issues related to foreign key value matching.</source>
      <translation type="unfinished">Column %1 in table %2 is referencing column %3 in table %4, but these columns have different data types: %5 vs. %6. This may cause issues related to foreign key value matching.</translation>
    </message>
  </context>
  <context>
    <name>ScriptingQtDbProxy</name>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="48"/>
      <source>No database available in current context, while called JavaScript&apos;s %1 command.</source>
      <translation>Nenhum banco de dados disponível, enquanto rodando JavaScript %1.</translation>
    </message>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="65"/>
      <source>Error from %1: %2</source>
      <translation>Erro de %1: %2</translation>
    </message>
  </context>
  <context>
    <name>SqlFileExecutor</name>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="56"/>
      <source>Could not execute SQL, because application has failed to start transaction: %1</source>
      <translation>Não foi possível executar SQL, porque a aplicação falhou ao iniciar a transação: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="87"/>
      <source>Execution from file cancelled. Any queries executed so far have been rolled back.</source>
      <translation>Execução do arquivo cancelada. Quaisquer consultas executadas até agora foram desfeitas.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="103"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Não foi possível abrir o arquivo &apos;%1&apos; para leitura: %2</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="150"/>
      <source>Could not execute SQL, because application has failed to commit the transaction: %1</source>
      <translation>Não foi possível executar SQL, porque o aplicativo falhou ao confirmar a transação: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="155"/>
      <source>Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.</source>
      <translation>Concluiu a execução de consultas %1 em %2 segundos. %3 não executado devido a erros.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="161"/>
      <source>Finished executing %1 queries in %2 seconds.</source>
      <translation>Terminou a consulta %1 em %2 segundos.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="168"/>
      <source>Could not execute SQL due to error.</source>
      <translation>Não foi possível executar SQL devido a um erro.</translation>
    </message>
  </context>
  <context>
    <name>SqlHistoryModel</name>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="32"/>
      <source>Database</source>
      <comment>sql history header</comment>
      <translation>Banco de dados</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="34"/>
      <source>Execution date</source>
      <comment>sql history header</comment>
      <translation>Data de execução</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="36"/>
      <source>Time spent</source>
      <comment>sql history header</comment>
      <translation>Tempo gasto</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="38"/>
      <source>Rows affected</source>
      <comment>sql history header</comment>
      <translation>Linhas afetadas</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="40"/>
      <source>SQL</source>
      <comment>sql history header</comment>
      <translation>SQL</translation>
    </message>
  </context>
  <context>
    <name>T</name>
    <message>
      <location filename="../db/abstractdb3.h" line="864"/>
      <source>Registered default collation on demand, under name: %1</source>
      <translation type="unfinished">Registered default collation on demand, under name: %1</translation>
    </message>
  </context>
  <context>
    <name>UpdateManager</name>
    <message>
      <location filename="../services/updatemanager.cpp" line="92"/>
      <source>Could not check for updates (%1).</source>
      <translation>Não foi possível verificar se há atualizações (%1).</translation>
    </message>
  </context>
</TS>
