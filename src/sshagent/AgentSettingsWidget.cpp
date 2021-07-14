/*
 *  Copyright (C) 2017 Toni Spets <toni.spets@iki.fi>
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AgentSettingsWidget.h"
#include "SSHAgent.h"
#include "ui_AgentSettingsWidget.h"

AgentSettingsWidget::AgentSettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::AgentSettingsWidget())
{
    m_ui->setupUi(this);
#ifndef Q_OS_WIN
    m_ui->useOpenSSHCheckBox->setVisible(false);
#else
    m_ui->sshAuthSockWidget->setVisible(false);
#endif
    m_ui->sshAuthSockMessageWidget->setVisible(sshAgent()->isEnabled());
    m_ui->sshAuthSockMessageWidget->setCloseButtonVisible(false);
    m_ui->sshAuthSockMessageWidget->setAutoHideTimeout(-1);
    connect(m_ui->enableSSHAgentCheckBox, SIGNAL(stateChanged(int)), SLOT(toggleSettingsEnabled()));
}

AgentSettingsWidget::~AgentSettingsWidget()
{
}

void AgentSettingsWidget::loadSettings()
{
    auto sshAgentEnabled = sshAgent()->isEnabled();

    m_ui->enableSSHAgentCheckBox->setChecked(sshAgentEnabled);
#ifdef Q_OS_WIN
    m_ui->useOpenSSHCheckBox->setChecked(sshAgent()->useOpenSSH());
#else
    auto sshAuthSock = sshAgent()->socketPath(false);
    auto sshAuthSockOverride = sshAgent()->authSockOverride();
    m_ui->sshAuthSockLabel->setText(sshAuthSock.isEmpty() ? tr("(empty)") : sshAuthSock);
    m_ui->sshAuthSockOverrideEdit->setText(sshAuthSockOverride);
#endif

    m_ui->sshAuthSockMessageWidget->setVisible(sshAgentEnabled);

    if (sshAgentEnabled) {
#ifndef Q_OS_WIN
        if (sshAuthSock.isEmpty() && sshAuthSockOverride.isEmpty()) {
            m_ui->sshAuthSockMessageWidget->showMessage(
                tr("No SSH Agent socket available. Either make sure SSH_AUTH_SOCK environment variable exists or set "
                   "an override."),
                MessageWidget::Warning);
            return;
        }
#endif
        QList<QSharedPointer<OpenSSHKey>> keys;
        if (sshAgent()->listIdentities(keys)) {
            m_ui->sshAuthSockMessageWidget->showMessage(tr("SSH Agent connection is working!"),
                                                        MessageWidget::Positive);
        } else {
            m_ui->sshAuthSockMessageWidget->showMessage(sshAgent()->errorString(), MessageWidget::Error);
        }
    }

    toggleSettingsEnabled();
}

void AgentSettingsWidget::saveSettings()
{
    auto sshAuthSockOverride = m_ui->sshAuthSockOverrideEdit->text();
    sshAgent()->setAuthSockOverride(sshAuthSockOverride);
#ifdef Q_OS_WIN
    sshAgent()->setUseOpenSSH(m_ui->useOpenSSHCheckBox->isChecked());
#endif
    sshAgent()->setEnabled(m_ui->enableSSHAgentCheckBox->isChecked());
}

void AgentSettingsWidget::toggleSettingsEnabled()
{
    m_ui->agentConfigPageBody->setEnabled(m_ui->enableSSHAgentCheckBox->isChecked());
}
