namespace SampleNetApp
{
    partial class frmMain
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
            this.btnSayHi = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnSayHi
            // 
            this.btnSayHi.Location = new System.Drawing.Point(95, 84);
            this.btnSayHi.Name = "btnSayHi";
            this.btnSayHi.Size = new System.Drawing.Size(75, 23);
            this.btnSayHi.TabIndex = 0;
            this.btnSayHi.Text = "SayHi";
            this.btnSayHi.UseVisualStyleBackColor = true;
            this.btnSayHi.Click += new System.EventHandler(this.btnSayHi_Click);
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(271, 208);
            this.Controls.Add(this.btnSayHi);
            this.Name = "frmMain";
            this.Text = "MainForm";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnSayHi;
    }
}

