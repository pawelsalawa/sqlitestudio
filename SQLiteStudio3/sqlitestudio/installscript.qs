function Component()
{
    component.loaded.connect(this, addRegisterFileCheckBox);
    component.fileTypes = ['db', 'db3', 'sqlite', 'sqlite3', 'sdb', 's3db'];
}

// called as soon as the component was loaded
addRegisterFileCheckBox = function()
{
    // don't show when updating or uninstalling
    if (installer.isInstaller()) {
        installer.addWizardPageItem(component, "RegisterFileCheckBoxForm", QInstaller.TargetDirectory);
		var checkbox = component.userInterface("RegisterFileCheckBoxForm").RegisterFileCheckBox;
        checkbox.text = checkbox.text + component.fileTypes.join(', ');
    }
}

// here we are creating the operation chain which will be processed at the real installation part later
Component.prototype.createOperations = function()
{
    // call default implementation to actually install the registeredfile
    component.createOperations();

    var isRegisterFileChecked = component.userInterface("RegisterFileCheckBoxForm").RegisterFileCheckBox.checked;
    if (installer.value("os") === "win") {
        var executable = "@TargetDir@/SQLiteStudio.exe";
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
