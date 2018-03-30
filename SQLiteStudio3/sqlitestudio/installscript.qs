function Component()
{
	if (installer.value("os") === "win") {
		component.loaded.connect(this, addOptionsCheckBoxForm);
		component.fileTypes = ['db', 'db3', 'sqlite', 'sqlite3', 'sdb', 's3db'];
	}
}

addOptionsCheckBoxForm = function()
{
    // don't show when updating or uninstalling
    if (installer.isInstaller()) {
        installer.addWizardPageItem(component, "OptionsCheckBoxForm", QInstaller.TargetDirectory);
		var form = component.userInterface("OptionsCheckBoxForm");
		
		var assocCheckBox = form.RegisterFileCheckBox;
        assocCheckBox.text = assocCheckBox.text + component.fileTypes.join(', ');

		var startMenuCheckbox = form.CreateStartMenuEntry;
		startMenuCheckbox.stateChanged.connect(this, function() {
			installer.setDefaultPageVisible(QInstaller.StartMenuSelection, startMenuCheckbox.checked);
		});
	}
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install the app
    component.createOperations();

    if (installer.value("os") === "win") {
		var form = component.userInterface("OptionsCheckBoxForm");
		var isRegisterFileChecked = form.RegisterFileCheckBox.checked;
		var isStartMenuEntryChecked = form.CreateStartMenuEntry.checked;
		var forAllUsersChecked = form.CreateStartMenuEntry.ForAllUsers.checked;
	
        var executable = "@TargetDir@/SQLiteStudio.exe";
		
		var linkPrefix = "@UserStartMenuProgramsPath@";
		if (forAllUsersChecked) {
			linkPrefix = "@AllUsersStartMenuProgramsPath@";
		}

		if (isRegisterFileChecked) {
			component.addOperation("CreateShortcut", executable, linkPrefix + "/@StartMenuDir@/SQLiteStudio.lnk",
				"workingDirectory=@TargetDir@", "iconPath=@TargetDir@/SQLiteStudio.exe",
				"iconId=0", "description=SQLiteStudio");
		}
			
		if (isRegisterFileChecked) {	
			component.fileTypes.forEach(function(fileType) {
				component.addOperation(
					"RegisterFileType",
					fileType,
					executable + " '%1'",
					"SQLite database",
					"application/octet-stream",
					executable + ",0",
					"ProgId=SQLiteStudio." + fileType
				);
			});
		}
    }
}
