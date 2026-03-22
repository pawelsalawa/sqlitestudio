<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr" sourcelanguage="en">
<context>
    <name>ErdChangeRegistryDialog</name>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="14"/>
        <source>Pending changes registry</source>
        <translation>Registre des modifications en attente</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="65"/>
        <source>Pending changes</source>
        <translation>Modifications en attente</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="105"/>
        <source>DDL preview</source>
        <translation>Aperçu DDL</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="143"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Shows the changes as they will be committed. Redundant or mutually cancelling steps are merged or removed from the list.&lt;br/&gt;When disabled, all individual undo and redo steps are shown.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Affiche les modifications telles qu'elles seront validées. Les étapes redondantes ou s'annulant mutuellement sont fusionnées ou supprimées de la liste.&lt;br/&gt;Lorsque désactivé, toutes les étapes d'annulation et de rétablissement individuelles sont affichées.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="146"/>
        <source>Show effective changes only</source>
        <translation>Afficher uniquement les modifications effectives</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="153"/>
        <source>&lt;html&gt;&lt;body&gt;Shows changes that do not modify the database schema, but only affect the diagram (for example, table position changes). &lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;body&gt;Affiche les modifications qui ne modifient pas le schéma de la base de données, mais affectent uniquement le diagramme (par exemple, les changements de position des tables). &lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="156"/>
        <source>Show non-schema changes</source>
        <translation>Afficher les modifications hors schéma</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.cpp" line="58"/>
        <source>-- This is a change applied only to the diagram. It has no database schema efects.</source>
        <comment>ERD editor</comment>
        <translation>-- Il s'agit d'une modification appliquée uniquement au diagramme. Elle n'a aucun effet sur le schéma de la base de données.</translation>
    </message>
</context>
<context>
    <name>ErdConfig</name>
    <message>
        <location filename="../erdconfig.ui" line="14"/>
        <source>Form</source>
        <translation>Formulaire</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="33"/>
        <source>Maximum number of tables for ERD editor</source>
        <translation>Nombre maximum de tables pour l'éditeur ERD</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="40"/>
        <source>Setting this value too high may cause the application to slow down or become unresponsive when opening the ERD editor.</source>
        <translation>Définir cette valeur trop haute peut ralentir l'application ou la rendre non réactive lors de l'ouverture de l'éditeur ERD.</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="49"/>
        <source>Erd.MaxTableLimit</source>
        <translation>Erd.MaxTableLimit</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="56"/>
        <source>Starts panning as soon as the Space key is pressed, without requiring a mouse button press.</source>
        <translation>Commence le défilement dès que la touche Espace est enfoncée, sans nécessiter d'appui sur un bouton de la souris.</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="59"/>
        <source>Pan view with Space only</source>
        <translation>Déplacer la vue avec Espace uniquement</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="62"/>
        <source>Erd.DragBySpace</source>
        <translation>Erd.DragBySpace</translation>
    </message>
</context>
<context>
    <name>ErdConnectionPanel</name>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="14"/>
        <source>Form</source>
        <translation>Formulaire</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="59"/>
        <source>Foreign key properties</source>
        <translation>Propriétés de la clé étrangère</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="83"/>
        <source>Composite relation (multiple columns)</source>
        <translation>Relation composite (plusieurs colonnes)</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="137"/>
        <source>Referencing (child) table:</source>
        <translation>Table référençante (enfant) :</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="156"/>
        <source>Referencing (child) column:</source>
        <translation>Colonne référençante (enfant) :</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="51"/>
        <source>ERD side panel for relation between tables &quot;%1&quot; and &quot;%2&quot; has uncommitted modifications.</source>
        <translation>Le panneau latéral ERD pour la relation entre les tables &quot;%1&quot; et &quot;%2&quot; comporte des modifications non validées.</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="57"/>
        <source>Apply changes to diagram</source>
        <comment>ERD editor</comment>
        <translation>Appliquer les modifications au diagramme</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="58"/>
        <source>Abort changes</source>
        <comment>ERD editor</comment>
        <translation>Annuler les modifications</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="240"/>
        <source>Modify relationship between &quot;%1&quot; and &quot;%2&quot;.</source>
        <translation>Modifier la relation entre &quot;%1&quot; et &quot;%2&quot;.</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="242"/>
        <source>Modify relationship between &quot;%1&quot; and &quot;%2&quot; - change target to &quot;%3&quot;.</source>
        <translation>Modifier la relation entre &quot;%1&quot; et &quot;%2&quot; - changer la cible en &quot;%3&quot;.</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="252"/>
        <source>Failed to execute DDL required for relation modification. Details: %1</source>
        <translation>Échec de l'exécution du DDL requis pour la modification de la relation. Détails : %1</translation>
    </message>
</context>
<context>
    <name>ErdEditorPlugin</name>
    <message>
        <location filename="../erdeditorplugin.cpp" line="20"/>
        <source>Open ERD editor</source>
        <translation>Ouvrir l'éditeur ERD</translation>
    </message>
    <message>
        <location filename="../erdeditorplugin.cpp" line="74"/>
        <source>ERD editor cannot open because the database contains %1 tables, exceeding the configured limit of %2 tables. You can increase this limit in the settings, but higher values may slow down or freeze the application.</source>
        <translation>L'éditeur ERD ne peut pas s'ouvrir car la base de données contient %1 tables, dépassant la limite configurée de %2 tables. Vous pouvez augmenter cette limite dans les paramètres, mais des valeurs plus élevées peuvent ralentir ou bloquer l'application.</translation>
    </message>
</context>
<context>
    <name>ErdScene</name>
    <message>
        <location filename="../scene/erdscene.cpp" line="530"/>
        <source>Delete multiple diagram elements.</source>
        <comment>ERD editor</comment>
        <translation>Supprimer plusieurs éléments du diagramme.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="547"/>
        <source>Failed to execute the undo DDL. Details: %1</source>
        <comment>ERD editor</comment>
        <translation>Échec de l'exécution du DDL d'annulation. Détails : %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="569"/>
        <source>Failed to execute the redo DDL. Details: %1</source>
        <comment>ERD editor</comment>
        <translation>Échec de l'exécution du DDL de rétablissement. Détails : %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="596"/>
        <source>Failed to execute DDL required for table deletion. Details: %1</source>
        <comment>ERD editor</comment>
        <translation>Échec de l'exécution du DDL requis pour la suppression de la table. Détails : %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="607"/>
        <source>Delete foreign key between &quot;%1&quot; and &quot;%2&quot;.</source>
        <translation>Supprimer la clé étrangère entre &quot;%1&quot; et &quot;%2&quot;.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="616"/>
        <source>Failed to execute DDL required for foreign key deletion. Details: %1</source>
        <translation>Échec de l'exécution du DDL requis pour la suppression de la clé étrangère. Détails : %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="824"/>
        <source>Arrange entities</source>
        <translation>Organiser les entités</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="825"/>
        <source>Are you sure you want to automatically arrange the entities on the diagram? This action will overwrite the current layout, and any manual adjustments will be lost.</source>
        <translation>Êtes-vous sûr de vouloir organiser automatiquement les entités sur le diagramme ? Cette action écrasera la disposition actuelle et tous les ajustements manuels seront perdus.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="845"/>
        <source>Change color of table &quot;%1&quot; to %2.</source>
        <translation>Changer la couleur de la table &quot;%1&quot; en %2.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="851"/>
        <source>Change color of multiple tables.</source>
        <translation>Changer la couleur de plusieurs tables.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="696"/>
        <source>Apply diagram layout</source>
        <translation>Appliquer la disposition du diagramme</translation>
    </message>
</context>
<context>
    <name>ErdTableWindow</name>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="29"/>
        <source>Apply changes to diagram</source>
        <comment>ERD editor</comment>
        <translation>Appliquer les modifications au diagramme</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="30"/>
        <source>Abort changes</source>
        <comment>ERD editor</comment>
        <translation>Annuler les modifications</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="49"/>
        <source>ERD side panel for table &quot;%1&quot; has uncommitted modifications.</source>
        <translation>Le panneau latéral ERD pour la table &quot;%1&quot; comporte des modifications non validées.</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="106"/>
        <source>Invalid table changes</source>
        <comment>ERD editor</comment>
        <translation>Modifications de table invalides</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="108"/>
        <source>&lt;b&gt;The table contains invalid changes&lt;/b&gt;</source>
        <comment>ERD editor</comment>
        <translation>&lt;b&gt;La table contient des modifications invalides&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="109"/>
        <source>Some of the changes you made cannot be applied because they contain errors.&lt;br&gt;&lt;br&gt;&lt;b&gt;Errors:&lt;/b&gt;&lt;br&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br&gt;&lt;br&gt;You can &lt;b&gt;return to editing&lt;/b&gt; and fix the problems, or &lt;b&gt;discard your changes&lt;/b&gt; and restore the previous state of the table.</source>
        <comment>ERD editor</comment>
        <translation>Certaines des modifications que vous avez effectuées ne peuvent pas être appliquées car elles contiennent des erreurs.&lt;br&gt;&lt;br&gt;&lt;b&gt;Erreurs :&lt;/b&gt;&lt;br&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br&gt;&lt;br&gt;Vous pouvez &lt;b&gt;retourner à l'édition&lt;/b&gt; et corriger les problèmes, ou &lt;b&gt;ignorer vos modifications&lt;/b&gt; et restaurer l'état précédent de la table.</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="118"/>
        <source>Fix errors</source>
        <comment>ERD editor</comment>
        <translation>Corriger les erreurs</translation>
    </message>
</context>
<context>
    <name>ErdView</name>
    <message>
        <location filename="../scene/erdview.cpp" line="323"/>
        <source>Cannot edit compound foreign keys this way. Such connections have to be edited through the side panel.</source>
        <comment>ERD editor</comment>
        <translation>Impossible de modifier les clés étrangères composites de cette façon. Ces connexions doivent être éditées via le panneau latéral.</translation>
    </message>
    <message>
        <location filename="../scene/erdview.cpp" line="633"/>
        <source>Move table &quot;%1&quot;</source>
        <translation>Déplacer la table &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../scene/erdview.cpp" line="639"/>
        <source>Move tables: %1</source>
        <translation>Déplacer les tables : %1</translation>
    </message>
</context>
<context>
    <name>ErdWindow</name>
    <message>
        <location filename="../erdwindow.ui" line="14"/>
        <source>Form</source>
        <translation>Formulaire</translation>
    </message>
    <message>
        <location filename="../erdwindow.ui" line="86"/>
        <source>Select an item to edit its properties</source>
        <translation>Sélectionner un élément pour modifier ses propriétés</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="164"/>
        <source>Cancels ongoing action</source>
        <comment>ERD editor</comment>
        <translation>Annule l'action en cours</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="165"/>
        <source>Create a table</source>
        <comment>ERD editor</comment>
        <translation>Créer une table</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="167"/>
        <location filename="../erdwindow.cpp" line="644"/>
        <source>Reload schema</source>
        <comment>ERD editor</comment>
        <translation>Recharger le schéma</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="168"/>
        <source>Commit all pending changes</source>
        <comment>ERD editor</comment>
        <translation>Valider toutes les modifications en attente</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="169"/>
        <source>Revert diagram to initial state</source>
        <comment>ERD editor</comment>
        <translation>Rétablir le diagramme à l'état initial</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="171"/>
        <source>Undo</source>
        <comment>ERD editor</comment>
        <translation>Annuler</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="172"/>
        <source>Redo</source>
        <comment>ERD editor</comment>
        <translation>Rétablir</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="188"/>
        <source>Create a table</source>
        <translation>Créer une table</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="201"/>
        <source>Select all</source>
        <translation>Sélectionner tout</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="310"/>
        <source>Filter items</source>
        <comment>ERD editor</comment>
        <translation>Filtrer les éléments</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="311"/>
        <source>Items that don’t match the filter will be dimmed.</source>
        <comment>ERD editor</comment>
        <translation>Les éléments ne correspondant pas au filtre seront estompés.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="447"/>
        <source>table name</source>
        <comment>ERD editor</comment>
        <translation>nom de la table</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="449"/>
        <source>column name</source>
        <comment>ERD editor</comment>
        <translation>nom de la colonne</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="569"/>
        <source>All changes have been successfully applied to the database.</source>
        <comment>ERD editor</comment>
        <translation>Toutes les modifications ont été appliquées avec succès à la base de données.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="575"/>
        <source>The changes were successfully committed. No modifications to the database schema were required.</source>
        <comment>ERD editor</comment>
        <translation>Les modifications ont été validées avec succès. Aucune modification du schéma de la base de données n'était nécessaire.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="583"/>
        <source>Failed to apply changes to the database. Details: %1</source>
        <comment>ERD editor</comment>
        <translation>Échec de l'application des modifications à la base de données. Détails : %1</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="645"/>
        <source>This action will discard all your pending changes and reload the diagram from the current database schema. The undo/redo history will be cleared. Do you want to proceed?</source>
        <translation>Cette action ignorera toutes vos modifications en attente et rechargera le diagramme depuis le schéma actuel de la base de données. L'historique d'annulation/rétablissement sera effacé. Voulez-vous continuer ?</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="826"/>
        <source>ERD window &quot;%1&quot; has uncommitted changes.</source>
        <translation>La fenêtre ERD &quot;%1&quot; comporte des modifications non validées.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="1138"/>
        <source>ERD editor (%1)</source>
        <translation>Éditeur ERD (%1)</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="1140"/>
        <source>ERD editor</source>
        <translation>Éditeur ERD</translation>
    </message>
    <message>
        <location filename="../erdwindow.ui" line="114"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;- Hold the Spacebar and drag with the mouse to pan the diagram freely without selecting any items.&lt;/p&gt;&lt;p&gt;- Use the mouse wheel to zoom in and out.&lt;/p&gt;&lt;p&gt;- Deselect the table (or click Commit in the side panel toolbar) to apply the side panel changes to the diagram.&lt;/p&gt;&lt;p&gt;- Press Esc (or click Rollback in the side panel toolbar) to discard the side panel changes.&lt;/p&gt;&lt;p&gt;- Double-Click on a table name or column to edit the name inline.&lt;/p&gt;&lt;p&gt;- Shift-Double-Click on a column to edit column details (datatype, constraints).&lt;/p&gt;&lt;p&gt;- To create a foreign key between table, click the middle mouse button on the table columns you want to connect.&lt;/p&gt;&lt;p&gt;- &lt;span style=&quot; font-weight:700;&quot;&gt;All diagram changes remain pending until you explicitly commit or roll them back&lt;/span&gt; using the toolbar buttons in the top-left corner of the diagram.&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;https://github.com/pawelsalawa/sqlitestudio/wiki/ERD-plugin-manual#usage&quot;&gt;&lt;span style=&quot; font-weight:700; text-decoration: underline; color:#058800;&quot;&gt;Learn more (click to open online help page)&lt;/span&gt;&lt;/a&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;- Maintenez la barre d&apos;espace et faites glisser la souris pour déplacer librement le diagramme sans sélectionner d&apos;éléments.&lt;/p&gt;&lt;p&gt;- Utilisez la molette de la souris pour zoomer et dézoomer.&lt;/p&gt;&lt;p&gt;- Désélectionnez la table (ou cliquez sur Valider dans la barre d&apos;outils du panneau latéral) pour appliquer les modifications du panneau latéral au diagramme.&lt;/p&gt;&lt;p&gt;- Appuyez sur Échap (ou cliquez sur Annuler dans la barre d&apos;outils du panneau latéral) pour ignorer les modifications du panneau latéral.&lt;/p&gt;&lt;p&gt;- Double-cliquez sur un nom de table ou de colonne pour modifier le nom en ligne.&lt;/p&gt;&lt;p&gt;- Maj+Double-clic sur une colonne pour modifier les détails de la colonne (type de données, contraintes).&lt;/p&gt;&lt;p&gt;- Pour créer une clé étrangère entre des tables, cliquez sur le bouton central de la souris sur les colonnes de la table que vous souhaitez connecter.&lt;/p&gt;&lt;p&gt;- &lt;span style=&quot; font-weight:700;&quot;&gt;Toutes les modifications du diagramme restent en attente jusqu&apos;à ce que vous les validiez ou les annuliez explicitement&lt;/span&gt; à l&apos;aide des boutons de la barre d&apos;outils dans le coin supérieur gauche du diagramme.&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;https://github.com/pawelsalawa/sqlitestudio/wiki/ERD-plugin-manual#usage&quot;&gt;&lt;span style=&quot; font-weight:700; text-decoration: underline; color:#058800;&quot;&gt;En savoir plus (cliquer pour ouvrir la page d&apos;aide en ligne)&lt;/span&gt;&lt;/a&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="180"/>
        <source>The number of changes pending for commit. Click to see details.</source>
        <comment>ERD editor</comment>
        <translation>Le nombre de modifications en attente de validation. Cliquez pour voir les détails.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="189"/>
        <source>Add a foreign key</source>
        <comment>ERD editor</comment>
        <translation>Ajouter une clé étrangère</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="191"/>
        <source>Delete selected items</source>
        <comment>ERD editor</comment>
        <translation>Supprimer les éléments sélectionnés</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="197"/>
        <source>Auto-arrange (local forces)</source>
        <comment>ERD editor</comment>
        <translation>Organisation automatique (forces locales)</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="198"/>
        <source>Auto-arrange (global balance)</source>
        <comment>ERD editor</comment>
        <translation>Organisation automatique (équilibre global)</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="212"/>
        <source>Set table color</source>
        <comment>ERD editor</comment>
        <translation>Définir la couleur de la table</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="240"/>
        <source>Use straight line</source>
        <comment>ERD editor</comment>
        <translation>Utiliser une ligne droite</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="241"/>
        <source>Use curvy line</source>
        <comment>ERD editor</comment>
        <translation>Utiliser une ligne courbe</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="242"/>
        <source>Use square line</source>
        <comment>ERD editor</comment>
        <translation>Utiliser une ligne en angle droit</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="253"/>
        <source>Choose line type</source>
        <comment>ERD editor</comment>
        <translation>Choisir le type de ligne</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../changes/erdchangedeleteentity.cpp" line="48"/>
        <source>Drop table &quot;%1&quot;.</source>
        <comment>ERD editor</comment>
        <translation>Supprimer la table &quot;%1&quot;.</translation>
    </message>
    <message>
        <location filename="../changes/erdchangemodifyentity.cpp" line="61"/>
        <source>Modify table &quot;%1&quot;.</source>
        <comment>ERD editor</comment>
        <translation>Modifier la table &quot;%1&quot;.</translation>
    </message>
    <message>
        <location filename="../changes/erdchangenewentity.cpp" line="54"/>
        <source>Create table &quot;%1&quot;.</source>
        <comment>ERD editor</comment>
        <translation>Créer la table &quot;%1&quot;.</translation>
    </message>
    <message>
        <location filename="../changes/erdeffectivechangemerger.cpp" line="31"/>
        <source>Failed to create in-memory databases required for change compacting.</source>
        <translation>Échec de la création des bases de données en mémoire requises pour la compaction des modifications.</translation>
    </message>
    <message>
        <location filename="../changes/erdeffectivechangemerger.cpp" line="399"/>
        <source>Drop tables: %1</source>
        <comment>ERD editor</comment>
        <translation>Supprimer les tables : %1</translation>
    </message>
    <message>
        <location filename="../scene/erdconnection.cpp" line="149"/>
        <source>Could not commit changes for finalized ERD connection.</source>
        <translation>Impossible de valider les modifications pour la connexion ERD finalisée.</translation>
    </message>
    <message>
        <location filename="../scene/erdconnection.cpp" line="155"/>
        <source>Update relationship from &quot;%1&quot;-&quot;%2&quot; to &quot;%1&quot;-&quot;%3&quot;.</source>
        <translation>Mettre à jour la relation de &quot;%1&quot;-&quot;%2&quot; vers &quot;%1&quot;-&quot;%3&quot;.</translation>
    </message>
    <message>
        <location filename="../scene/erdconnection.cpp" line="157"/>
        <source>Create relationship between &quot;%1&quot; and &quot;%2&quot;.</source>
        <translation>Créer une relation entre &quot;%1&quot; et &quot;%2&quot;.</translation>
    </message>
    <message>
        <location filename="../changes/erdchangemoveentity.cpp" line="28"/>
        <source>Move table &quot;%1&quot;</source>
        <translation>Déplacer la table &quot;%1&quot;</translation>
    </message>
</context>
</TS>
