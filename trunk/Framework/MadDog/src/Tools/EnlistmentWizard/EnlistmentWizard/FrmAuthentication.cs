using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Tools.EnlistmentWizard.UI
{
    public partial class FrmAuthentication : Form
    {
        private Boolean isOK = false;
        private String accountName = null;
        private String password = null;

        public delegate void EventHandler(object sender, System.EventArgs e);
        public event EventHandler OnFinish;
        
        public String AccountName
        {
            get
            {
                return accountName;
            }
        }

        public String Password
        {
            get
            {
                return password;
            }
        }

        public Boolean IsOK
        {
            get
            {
                return this.isOK;
            }
        }

        public FrmAuthentication()
        {
            InitializeComponent();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this.accountName = txtAccountName.Text;
            this.password = txtPassword.Text;
            this.isOK = true;
            this.Dispose();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Dispose();
        }
    }
}
