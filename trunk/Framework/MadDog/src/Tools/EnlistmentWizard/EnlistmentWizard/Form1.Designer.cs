namespace Tools.EnlistmentWizard.UI
{
    partial class FrmEnlistSetting
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lblDDKPath = new System.Windows.Forms.Label();
            this.txtDDKPath = new System.Windows.Forms.TextBox();
            this.lblEnlistmentPath = new System.Windows.Forms.Label();
            this.txtEnlistmentPath = new System.Windows.Forms.TextBox();
            this.btnEnlist = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.progressEnlistment = new System.Windows.Forms.ProgressBar();
            this.btnSelectEnlistPath = new System.Windows.Forms.Button();
            this.btnSelectDDKPath = new System.Windows.Forms.Button();
            this.lblProgress = new System.Windows.Forms.Label();
            this.btnShowDetails = new System.Windows.Forms.Button();
            this.lstFile = new System.Windows.Forms.ListBox();
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.SuspendLayout();
            // 
            // lblDDKPath
            // 
            this.lblDDKPath.AutoSize = true;
            this.lblDDKPath.Location = new System.Drawing.Point(15, 81);
            this.lblDDKPath.Name = "lblDDKPath";
            this.lblDDKPath.Size = new System.Drawing.Size(77, 13);
            this.lblDDKPath.TabIndex = 0;
            this.lblDDKPath.Text = "WinDDK Path:";
            // 
            // txtDDKPath
            // 
            this.txtDDKPath.Location = new System.Drawing.Point(133, 81);
            this.txtDDKPath.Name = "txtDDKPath";
            this.txtDDKPath.Size = new System.Drawing.Size(258, 20);
            this.txtDDKPath.TabIndex = 3;
            // 
            // lblEnlistmentPath
            // 
            this.lblEnlistmentPath.AutoSize = true;
            this.lblEnlistmentPath.Location = new System.Drawing.Point(15, 39);
            this.lblEnlistmentPath.Name = "lblEnlistmentPath";
            this.lblEnlistmentPath.Size = new System.Drawing.Size(83, 13);
            this.lblEnlistmentPath.TabIndex = 2;
            this.lblEnlistmentPath.Text = "Enlistment Path:";
            // 
            // txtEnlistmentPath
            // 
            this.txtEnlistmentPath.Location = new System.Drawing.Point(133, 31);
            this.txtEnlistmentPath.Name = "txtEnlistmentPath";
            this.txtEnlistmentPath.Size = new System.Drawing.Size(258, 20);
            this.txtEnlistmentPath.TabIndex = 1;
            // 
            // btnEnlist
            // 
            this.btnEnlist.Location = new System.Drawing.Point(239, 121);
            this.btnEnlist.Name = "btnEnlist";
            this.btnEnlist.Size = new System.Drawing.Size(100, 23);
            this.btnEnlist.TabIndex = 5;
            this.btnEnlist.Text = "Enlist";
            this.btnEnlist.UseVisualStyleBackColor = true;
            this.btnEnlist.Click += new System.EventHandler(this.btnEnlist_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(345, 121);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(100, 23);
            this.btnCancel.TabIndex = 6;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // progressEnlistment
            // 
            this.progressEnlistment.Location = new System.Drawing.Point(81, 181);
            this.progressEnlistment.Name = "progressEnlistment";
            this.progressEnlistment.Size = new System.Drawing.Size(357, 23);
            this.progressEnlistment.TabIndex = 6;
            // 
            // btnSelectEnlistPath
            // 
            this.btnSelectEnlistPath.Location = new System.Drawing.Point(397, 28);
            this.btnSelectEnlistPath.Name = "btnSelectEnlistPath";
            this.btnSelectEnlistPath.Size = new System.Drawing.Size(41, 23);
            this.btnSelectEnlistPath.TabIndex = 2;
            this.btnSelectEnlistPath.Text = "...";
            this.btnSelectEnlistPath.UseVisualStyleBackColor = true;
            this.btnSelectEnlistPath.Click += new System.EventHandler(this.btnSelectEnlistPath_Click);
            // 
            // btnSelectDDKPath
            // 
            this.btnSelectDDKPath.Location = new System.Drawing.Point(397, 78);
            this.btnSelectDDKPath.Name = "btnSelectDDKPath";
            this.btnSelectDDKPath.Size = new System.Drawing.Size(41, 23);
            this.btnSelectDDKPath.TabIndex = 4;
            this.btnSelectDDKPath.Text = "...";
            this.btnSelectDDKPath.UseVisualStyleBackColor = true;
            this.btnSelectDDKPath.Click += new System.EventHandler(this.btnSelectDDKPath_Click);
            // 
            // lblProgress
            // 
            this.lblProgress.AutoSize = true;
            this.lblProgress.Location = new System.Drawing.Point(15, 181);
            this.lblProgress.Name = "lblProgress";
            this.lblProgress.Size = new System.Drawing.Size(51, 13);
            this.lblProgress.TabIndex = 8;
            this.lblProgress.Text = "Progress:";
            // 
            // btnShowDetails
            // 
            this.btnShowDetails.Enabled = false;
            this.btnShowDetails.Location = new System.Drawing.Point(133, 121);
            this.btnShowDetails.Name = "btnShowDetails";
            this.btnShowDetails.Size = new System.Drawing.Size(100, 23);
            this.btnShowDetails.TabIndex = 9;
            this.btnShowDetails.Text = "Show Details >>";
            this.btnShowDetails.UseVisualStyleBackColor = true;
            this.btnShowDetails.Click += new System.EventHandler(this.btnShowDetails_Click);
            // 
            // lstFile
            // 
            this.lstFile.FormattingEnabled = true;
            this.lstFile.Location = new System.Drawing.Point(81, 218);
            this.lstFile.Name = "lstFile";
            this.lstFile.Size = new System.Drawing.Size(357, 134);
            this.lstFile.TabIndex = 10;
            // 
            // FrmEnlistSetting
            // 
            this.AcceptButton = this.btnEnlist;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(457, 154);
            this.Controls.Add(this.lstFile);
            this.Controls.Add(this.btnShowDetails);
            this.Controls.Add(this.lblProgress);
            this.Controls.Add(this.btnSelectDDKPath);
            this.Controls.Add(this.btnSelectEnlistPath);
            this.Controls.Add(this.progressEnlistment);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnEnlist);
            this.Controls.Add(this.txtEnlistmentPath);
            this.Controls.Add(this.lblEnlistmentPath);
            this.Controls.Add(this.txtDDKPath);
            this.Controls.Add(this.lblDDKPath);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "FrmEnlistSetting";
            this.Text = "Enlistment Setting";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblDDKPath;
        private System.Windows.Forms.TextBox txtDDKPath;
        private System.Windows.Forms.Label lblEnlistmentPath;
        private System.Windows.Forms.TextBox txtEnlistmentPath;
        private System.Windows.Forms.Button btnEnlist;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.ProgressBar progressEnlistment;
        private System.Windows.Forms.Button btnSelectEnlistPath;
        private System.Windows.Forms.Button btnSelectDDKPath;
        private System.Windows.Forms.Label lblProgress;
        private System.Windows.Forms.Button btnShowDetails;
        private System.Windows.Forms.ListBox lstFile;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
    }
}

