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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Diagnostics;
using System.Windows.Threading;

namespace Gui3
{

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void txt_ClientXML_TextChanged(object sender, RoutedEventArgs e)
        {
            saveLastFileName(txt_ClientXML);
        }
        private void txt_DataShapeXML_TextChanged(object sender, RoutedEventArgs e)
        {
            saveLastFileName(txt_DataShapeXML);
        }
        private void txt_DataSetXML_TextChanged(object sender, RoutedEventArgs e)
        {
            saveLastFileName(txt_DataSetXML);
        }
        private void txt_EngineXML_TextChanged(object sender, RoutedEventArgs e)
        {
            saveLastFileName(txt_EngineXML);
        }
        private void txt_SaveEnginePid_TextChanged(object sender, RoutedEventArgs e)
        {
            saveLastFileName(txt_SaveEnginePid);
        }

        private void btn_ClientXML_Click(object sender, RoutedEventArgs e)
        {
            txt_ClientXML.Text = getDlgFileName();
        }
        private void btn_DataShapeXML_Click(object sender, RoutedEventArgs e)
        {
            txt_DataShapeXML.Text = getDlgFileName();
        }
        private void btn_DataSetXML_Click(object sender, RoutedEventArgs e)
        {
            txt_DataSetXML.Text = getDlgFileName();
        }
        private void btn_EngineXML_Click(object sender, RoutedEventArgs e)
        {
            txt_EngineXML.Text = getDlgFileName();
        }
        private void btn_EnginePid_Click(object sender, RoutedEventArgs e)
        {
        }

        //----------- Utilities ----------------
        string getDlgFileName()
        {
            // Create OpenFileDialog 
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            // Set filter for file extension and default file extension 
            dlg.DefaultExt = ".xml";
            dlg.Filter = "XML Files (*.xml)|*.xml|All Files (*.*)|*.*";

            // Display OpenFileDialog by calling ShowDialog method 
            Nullable<bool> result = dlg.ShowDialog();

            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                // Open document 
                return dlg.FileName;
            }
            return "";
        }
        void saveLastFileName(TextBox tb)
        {
            string fname = tb.Name + ".last";
            string[] lines = { tb.Text };
            System.IO.File.WriteAllLines(@fname, lines);
        }
        void loadLastFileName(TextBox tb)
        {
            string fname = tb.Name + ".last";
            try
            {
                tb.Text = File.ReadAllText(fname);
            }
            catch (System.IO.IOException e)
            {

            }
        }
        private void MainWindow_Activated(object sender, System.EventArgs e)
        {
            loadLastFileName(txt_ClientXML);
            loadLastFileName(txt_DataShapeXML);
            loadLastFileName(txt_DataSetXML);
            loadLastFileName(txt_EngineXML);
        }

        private void btn_Go_Click(object sender, RoutedEventArgs e)
        {
            string exepath = System.AppDomain.CurrentDomain.BaseDirectory + "../../zzz.bat";
            string exeargs = ((bool)(rb_ActionTrain.IsChecked) ? "Train": (bool)(rb_ActionInfer.IsChecked) ? "Infer" : "Both");
            exeargs = exeargs + " " + txt_SimulationId.Text.Replace("\r\n", string.Empty) + " " + txt_ClientXML.Text.Replace("\r\n", string.Empty) + " " + txt_DataShapeXML.Text.Replace("\r\n", string.Empty) + " " + txt_DataSetXML.Text.Replace("\r\n", string.Empty) + " " + txt_EngineXML.Text.Replace("\r\n", string.Empty);// + " "+ txt_SaveEnginePid.Text.Replace("\r\n", string.Empty);
            string fullexepath = exepath;// + " " + exeargs;
            Process.Start(exepath, exeargs);
        }
        //--------------------------------------


    }
}

