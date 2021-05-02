using System;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;

namespace SampleNetApp
{
    public partial class frmMain : Form
    {
        public frmMain()
        {
            InitializeComponent();
        }

        private void btnSayHi_Click(object sender, EventArgs e)
        {
            
            MessageBox.Show("Hello world from .Net App");

        }
    }
}
