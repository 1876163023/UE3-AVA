using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MemoryProfiler
{
    public partial class Form2 : Form
    {
        public Form2()
        {
            InitializeComponent();
        }

        private void ClearAllTagsButton_Click(object sender, EventArgs e)
        {
            for( int Idx = 0; Idx < MemTagCheckedListBox.Items.Count; Idx++ )
            {
                MemTagCheckedListBox.SetItemChecked(Idx, false);
            } 
        }
    }
}