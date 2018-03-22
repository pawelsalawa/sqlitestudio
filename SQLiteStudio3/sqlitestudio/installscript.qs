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
		var assocCheckBox = component.userInterface("OptionsCheckBoxForm").RegisterFileCheckBox;
        assocCheckBox.text = assocCheckBox.text + component.fileTypes.join(', ');

		var startMenuCheckbox = component.userInterface("OptionsCheckBoxForm").CreateStartMenuEntryCheckBox;
		startMenuCheckbox.stateChanged.connect(this, function() {
			console.log("Start menu CB state changed: " +  startMenuCheckbox.checked);
			installer.setDefaultPageVisible(QInstaller.StartMenuSelection, startMenuCheckbox.checked);
		});
	}
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install the app
    component.createOperations();

	var theForm = component.userInterface("OptionsCheckBoxForm");
    var isRegisterFileChecked = theForm.RegisterFileCheckBox.checked;
	var isStartMenuEntryChecked = theForm.CreateStartMenuEntryCheckBox.checked;
    if (installer.value("os") === "win") {
        var executable = "@TargetDir@/SQLiteStudio.exe";

		if (isRegisterFileChecked) {
			component.addOperation("CreateShortcut", executable, "@StartMenuDir@/SQLiteStudio.lnk",
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
