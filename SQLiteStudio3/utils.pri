defineTest(copy_dir) {
    #message("copying $$absolute_path($$1) to $$absolute_path($$2)");
    unix: {
            system(cp -R $$quote($$1) $$quote($$2))
    }
    win32: {
            system(xcopy \"$$quote($$1)\" \"$$quote($$2)\" /s /e /y /q /i)
    }
}

defineTest(copy_file) {
    #message("copying $$absolute_path($$1) to $$absolute_path($$2)");
    unix: {
            system(cp $$quote($$1) $$quote($$2))
    }
    win32: {
            system(copy \"$$quote($$1)\" \"$$quote($$2)\" /y)
    }
}

# This would be better way, but targets defined inside of test function are not visible outside
# and they cannot be exported. I need to find another way to do this.
#
#defineTest(copy_file) {
#    message("copying $$absolute_path($$1) to $$absolute_path($$2)");
#    unix: {
#            copy_target.commands = cp $$quote($$absolute_path($$1)) $$quote($$absolute_path($$2))
#    }
#    win32: {
#            copy_target.commands = copy $$quote($$absolute_path($$1)) $$quote($$absolute_path($$2)) /y
#    }
#    QMAKE_EXTRA_TARGETS += copy_target
#    PRE_TARGETDEPS += copy_target
#    export(PRE_TARGETDEPS)
#    export(QMAKE_EXTRA_TARGETS)
#}
