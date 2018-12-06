using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Gui3
{
    /// <summary>
    /// Interaction logic for XMLeditor.xaml
    /// </summary>
    public partial class XMLeditor : Window
    {

        TextBox caller;

        public XMLeditor(TextBox caller_)
        {
            caller = caller_;
            InitializeComponent();
            lbFileName.Content = caller.Text.Replace("\r\n", string.Empty);
        }

        private void rtbXMLcontent_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void btnSaveClose_Click(object sender, RoutedEventArgs e)
        {
            System.IO.StreamWriter sw = new System.IO.StreamWriter(lbFileName.Content.ToString());
            string richText = new TextRange(rtbXMLcontent.Document.ContentStart, rtbXMLcontent.Document.ContentEnd).Text;
            sw.Write(richText);
            sw.Close();
            caller.ToolTip = richText;
            this.Close();
        }

        private void XMLeditor_Activated(object sender, System.EventArgs e)
        {
            

        }
    }
}
