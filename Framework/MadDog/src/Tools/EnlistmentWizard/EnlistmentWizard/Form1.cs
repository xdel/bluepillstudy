using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Tools.EnlistmentWizard.Core;
using Core.Svn;
using System.IO;
using Tools.EnlistmentWizard.Core.Util;

namespace Tools.EnlistmentWizard.UI
{
    public partial class FrmEnlistSetting : Form
    {
        private Boolean isShowDetail = false;
        public FrmEnlistSetting()
        {
            InitializeComponent();
            InitValues();
        }
        private void InitValues()
        {
            txtDDKPath.Text = GlobalEnvironment.DDKHome;
        }

        private void btnShowDetails_Click(object sender, EventArgs e)
        {
            if (!isShowDetail)
            {
                this.Height = 415;
                btnShowDetails.Text = "Show Details <<";
                isShowDetail = true;
                
            }
            else
            {
                this.Height = 186;
                btnShowDetails.Text = "Show Details >>";
                isShowDetail = false;
            }
        }

        private void btnEnlist_Click(object sender, EventArgs e)
        {
            Enlist();
        }

        private void Enlist()
        {
            //Step 1.Set Global Environments
            GlobalEnvironment.DDKHome = txtDDKPath.Text;

            //Step 2.Enlist From Server
            String enlistLocalPath = txtEnlistmentPath.Text.TrimEnd('\\');
            if (!Directory.Exists(enlistLocalPath))
            {
                try
                {
                    Directory.CreateDirectory(enlistLocalPath);
                }
                catch
                {
                    return;
                }
            }
            SvnService.SvnCheckoutWorkspace(enlistLocalPath);

            //Step 3.Modify Razzle.bat
            String razzleFile = enlistLocalPath + RazzleFilePath.RAZZLETEMPLATE_BAT_FILEPATH;
            String destRazzleBatName = String.Format(@"\Razzle_{0}.bat", GlobalEnvironment.RazzleCount);
            String destRazzleBatPathName = Environment.GetFolderPath(Environment.SpecialFolder.System) +
                destRazzleBatName;
            StreamReader sr = new StreamReader(razzleFile);
            String content = sr.ReadToEnd();
            sr.Close();

            //Step 3.1 Replace %%ENLISTMENT_PROJ_ROOT%% in Razzle.bat
            content = content.Replace(TemplateStrings.LOCAL_ENLISTMENT_PROJ_ROOT_STRING, enlistLocalPath +@"\MadDog");
            //Step 3.2 Replace %%ENLISTMENT_PROJ_ROOT%% in Razzle.bat
            content=content.Replace(TemplateStrings.LOCAL_RAZZLE_INDEX,GlobalEnvironment.RazzleCount.ToString());

            StreamWriter sw = new StreamWriter(destRazzleBatPathName);
            sw.Write(content);
            sw.Close();

            //Step 4.Create Shortcut on the desktop.
            String shortCutFileName = Environment.GetFolderPath(System.Environment.SpecialFolder.Desktop) + 
                String.Format(@"\Razzle_{0}.lnk", GlobalEnvironment.RazzleCount);
            //Step 4.1 Set Razzle Build Env x86 Checked in Default.
            ShortcutHelper.CreateShortcut(shortCutFileName,
                destRazzleBatPathName, 
                RazzleFilePath.RAZZLE_BAT_DEFAULT_PARAMETER,
                enlistLocalPath);

            //Step 5. Increment Razzle_Count Value
            GlobalEnvironment.IncreaseRazzleCount();
        }

        private void btnSelectEnlistPath_Click(object sender, EventArgs e)
        {
            folderBrowserDialog1.SelectedPath = txtEnlistmentPath.Text;
            folderBrowserDialog1.ShowDialog(this);
            txtEnlistmentPath.Text = folderBrowserDialog1.SelectedPath;
        }

        private void btnSelectDDKPath_Click(object sender, EventArgs e)
        {
            folderBrowserDialog1.SelectedPath = txtEnlistmentPath.Text;
            folderBrowserDialog1.ShowDialog(this);
            txtEnlistmentPath.Text = folderBrowserDialog1.SelectedPath;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Dispose();
        }

    }
}
