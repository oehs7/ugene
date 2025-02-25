/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2021 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __OPENCL_SUPPORT_SETTINGS_CONTROLLER__
#define __OPENCL_SUPPORT_SETTINGS_CONTROLLER__

#include <QLabel>
#include <QRadioButton>
#include <QVector>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

#define OpenCLSupportSettingsPageId QString("oclss")

class OpenCLSupportSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    OpenCLSupportSettingsPageController(const QString &_displayMsg, QObject *p = 0);

    virtual AppSettingsGUIPageState *getSavedState();

    virtual void saveState(AppSettingsGUIPageState *s);

    virtual AppSettingsGUIPageWidget *createWidget(AppSettingsGUIPageState *state);

    const QString &getHelpPageId() const {
        return helpPageId;
    };

private:
    QString displayMsg;
    static const QString helpPageId;
};

class OpenCLSupportSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    OpenCLSupportSettingsPageState(const QString &name);

    const QString &getEnabledGpuName() const;

private:
    QString enabledGpuName;
};

class OpenCLSupportSettingsPageWidget : public AppSettingsGUIPageWidget {
    Q_OBJECT
public:
    OpenCLSupportSettingsPageWidget(const QString &_msg, OpenCLSupportSettingsPageController *ctrl);

    virtual void setState(AppSettingsGUIPageState *state);
    virtual AppSettingsGUIPageState *getState(QString &err) const;

private:
    QString onlyMsg;
    QMap<QString, QRadioButton *> gpuRadioButtons;
};

}  // namespace U2

#endif  //__OPENCL_SUPPORT_SETTINGS_CONTROLLER__
