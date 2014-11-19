#ifndef DBPLUGINOPTION_H
#define DBPLUGINOPTION_H

#include <QString>
#include <QVariant>

/**
 * @brief Database plugin connection options.
 *
 * It is used to identify connection options that the DbPlugin implementation needs
 * for the plugin to be configured by the user in the DbDialog.
 *
 * Single DbPluginOption represents in DbDialog:
 * <ul>
 * <li>single QLabel with text set to DbPluginOption::label,</li>
 * <li>an input widget, that depends on DbPluginOption::type.</li>
 * </ul>
 *
 * The input widget is different for different data type expected for the option.
 * See DbPluginOption::Type for details.
 *
 * After user entered his values for options in DbDialog, they are passed
 * to the DbPlugin::getInstance() and later to the Db::init(). Options are passed
 * as key-value pairs, given the DbPluginOption::key and value specified by the user
 * in DbDialog.
 */
struct DbPluginOption
{
    /**
     * @brief Option data type
     *
     * Determinates what kind of input widget will be added to DbDialog.
     */
    enum Type
    {
        STRING = 0, /**< QLineEdit will be added. */
        INT = 1, /**< QSpinBox will be added */
        BOOL = 2, /**< QCheckBox will be added */
        DOUBLE = 3, /**< QDoubleSpinBox will be added */
        FILE = 4, /**< QLineEdit will be added */
        PASSWORD = 5, /**< QLineEdit with value masking will be added */
        CHOICE = 6 /**< QComboBox will be added */
    };

    /**
     * @brief Name for the key in QHash collected from options in DbDialog and
     * later passed to DbPlugin::getInstance().
     */
    QString key;

    /**
     * @brief Label text to be used in DbDialog to inform user what is this option.
     */
    QString label;

    /**
     * @brief Optional tooltip to show for added widget.
     */
    QString toolTip;

    /**
     * @brief Optional placeholder text for QLineEdit widget.
     */
    QString placeholderText;

    /**
     * @brief List of values for QComboBox.
     */
    QStringList choiceValues;

    /**
     * @brief Default value to be set in the editor widget.
     */
    QVariant defaultValue;

    /**
     * @brief Indicates if the combobox should be read only or writable.
     */
    bool choiceReadOnly = true;

    /**
     * @brief Minimum value for numeric editors (double or int).
     */
    QVariant minValue;

    /**
     * @brief Maximum value for numeric editors (double or int).
     */
    QVariant maxValue;

    /**
     * @brief Expected data type for the option.
     */
    Type type;
};

#endif // DBPLUGINOPTION_H
