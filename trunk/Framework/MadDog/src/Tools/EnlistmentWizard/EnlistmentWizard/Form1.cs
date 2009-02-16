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

        private String accountName;
        private String password;
        private void btnEnlist_Click(object sender, EventArgs e)
        {
            //Step 1.Show the authentication window
            FrmAuthentication authWnd = new FrmAuthentication();
            authWnd.OnFinish += new FrmAuthentication.EventHandler(authWnd_OnFinish);
            authWnd.Show(this);
            
        }

        private void authWnd_OnFinish(object sender, EventArgs e)
        {
            FrmAuthentication authWnd = (FrmAuthentication)sender;
            if (authWnd.IsOK)
            {
                //Step 2. Start to enlist.
                this.password = authWnd.Password;
                this.accountName = authWnd.AccountName;
                Enlist();
                //Step 3. Finialize
                this.Dispose();
            }
        }

        private void Enlist()
        {
            //Step 1.Set Global Environments
            GlobalEnvironment.DDKHome = txtDDKPath.Text;

            //Step 2.Enlist From Server
            String enlistLocalPath = txtEnlistmentPath.Text.TrimEnd('\\');
            String winDDKHomePath = txtDDKPath.Text.TrimEnd('\\');
            if (!Directory.Exists(enlistLocalPath))
            {
                try
                {
                    Directory.CreateDirectory(enlistLocalPath);
                }
                catch
                {
                    MessageBox.Show("Invalid Path!");
                    return;
                }
            }
            //Check if it exits normally
            Boolean runSuccessfully = SvnService.SvnCheckoutWorkspace(this.accountName,this.password,enlistLocalPath);
            if (!runSuccessfully)
            {
                MessageBox.Show("Svn Checkout Failed");
                return;
            }

            //Step 3.Modify Razzle.bat
            String razzleFile = enlistLocalPath + RazzleFilePath.RAZZLETEMPLATE_BAT_FILEPATH;
            int razzleIndex = GlobalEnvironment.RazzleCount + 1;
            String destRazzleBatName = String.Format(@"\Razzle_{0}.bat",razzleIndex);
            String destRazzleBatPathName = Environment.GetFolderPath(Environment.SpecialFolder.System) +
                destRazzleBatName;
            StreamReader sr = new StreamReader(razzleFile);
            String content = sr.ReadToEnd();
            sr.Close();

            //Step 3.1 Replace %%ENLISTMENT_PROJ_ROOT%% in Razzle.bat
            content = content.Replace(TemplateStrings.LOCAL_ENLISTMENT_PROJ_ROOT_STRING, enlistLocalPath +@"\MadDog");
            //Step 3.2 Replace %%ENLISTMENT_PROJ_ROOT%% in Razzle.bat
            content = content.Replace(TemplateStrings.LOCAL_RAZZLE_INDEX, razzleIndex.ToString());
            //Step 3.3 Replace %%WINDDK_HOME%% in Razzle.bat
            content = content.Replace(TemplateStrings.LOCAL_WINDDKHOME_STRING, winDDKHomePath+"\\");

            StreamWriter sw = new StreamWriter(destRazzleBatPathName);
            sw.Write(content);
            sw.Close();

            //Step 4.Create Shortcut on the desktop.
            String shortCutFileName = Environment.GetFolderPath(System.Environment.SpecialFolder.Desktop) +
                String.Format(@"\Razzle_{0}.lnk", razzleIndex);
            //Step 4.1 Set Razzle Build Env x86 Checked in Default.
            ShortcutHelper.CreateShortcut(shortCutFileName,
                destRazzleBatPathName, 
                RazzleFilePath.RAZZLE_BAT_DEFAULT_PARAMETER,
                enlistLocalPath);

            //Step 5. Increment Razzle_Count Value
            GlobalEnvironment.IncreaseRazzleCount();

            MessageBox.Show("Enlist successfully");
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
