function Controller() {
    if (installer.isInstaller()) {
        installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
    }
}
