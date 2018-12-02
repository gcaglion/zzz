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
using System.Runtime.InteropServices;
using System.ComponentModel;

namespace Gui3
{

    public partial class sWorkerParms
    {
        public int what;
        public int simulationId;
        public StringBuilder clientXML;
        public StringBuilder dataShapeXML;
        public StringBuilder dataSetXML;
        public StringBuilder engineXML;
        public int savedEnginePid;
    };

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {


        public MainWindow()
        {
            InitializeComponent();
        }

        private void txt_SimulationId_TextChanged(object sender, RoutedEventArgs e)
        {
            saveLastFileName(txt_SimulationId);
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
        //--
        private void btn_SimulationId_Click(object sender, RoutedEventArgs e) {}
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
        private void btn_EnginePid_Click(object sender, RoutedEventArgs e) {}

        //----------- Utilities ----------------

        public delegate void ReportProgressDelegate(int progress, object state);

        [DllImport("Forecaster.dll", CallingConvention = CallingConvention.Cdecl)] public static extern int _trainClient(int simulationId_, StringBuilder clientXMLfile_, StringBuilder shapeXMLfile_, StringBuilder trainXMLfile_, StringBuilder engineXMLfile_, ReportProgressDelegate progressDel);
        [DllImport("Forecaster.dll", CallingConvention = CallingConvention.Cdecl)] public static extern int _inferClient(int simulationId_, StringBuilder clientXMLfile_, StringBuilder shapeXMLfile_, StringBuilder inferXMLfile_, StringBuilder engineXMLfile_, int savedEnginePid_, ReportProgressDelegate progressDel);
        [DllImport("Forecaster.dll", CallingConvention = CallingConvention.Cdecl)] public static extern int _bothClient(int simulationId_, StringBuilder clientXMLfile_, StringBuilder shapeXMLfile_, StringBuilder bothXMLfile_, StringBuilder engineXMLfile_, ReportProgressDelegate progressDel);
        //--
   
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
            loadLastFileName(txt_SimulationId);
            loadLastFileName(txt_ClientXML);
            loadLastFileName(txt_DataShapeXML);
            loadLastFileName(txt_DataSetXML);
            loadLastFileName(txt_EngineXML);
        }
        //--------------------------------------

        //-- external call to zzz.bat
        private void btn_Go_Click(object sender, RoutedEventArgs e)
        {
           Environment.SetEnvironmentVariable("PATH", "D:/app/oracle/product/12.1.0/dbhome_1/oci/lib/msvc/vc14;D:/app/oracle/product/12.1.0/dbhome_1/bin;C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/bin");

            sWorkerParms wp= new sWorkerParms();

            if ((bool)(rb_ActionTrain.IsChecked)) wp.what = 1;  // Train
            if ((bool)(rb_ActionInfer.IsChecked)) wp.what = 2;  // Infer
            if ((bool)(rb_ActionBoth.IsChecked))  wp.what = 3;  // Both

            wp.simulationId = Convert.ToInt32(txt_SimulationId.Text.Replace("\r\n", string.Empty)); 
            wp.clientXML = new StringBuilder(txt_ClientXML.Text).Replace("\r\n", string.Empty);
            wp.dataShapeXML = new StringBuilder(txt_DataShapeXML.Text).Replace("\r\n", string.Empty);
            wp.dataSetXML = new StringBuilder(txt_DataSetXML.Text).Replace("\r\n", string.Empty);
            wp.engineXML = new StringBuilder(txt_EngineXML.Text).Replace("\r\n", string.Empty);
            wp.savedEnginePid = Convert.ToInt32(txt_SaveEnginePid.Text.Replace("\r\n", string.Empty));

            BackgroundWorker worker = new BackgroundWorker();
            worker.WorkerReportsProgress = true;
            worker.DoWork += worker_DoWork;
            worker.ProgressChanged += worker_ProgressChanged;
            worker.RunWorkerCompleted += worker_RunWorkerCompleted;
            worker.RunWorkerAsync(wp);

            
        }

        private void btn_Cancel_Click(object sender, RoutedEventArgs e) { }
        
        //================================ WORKER STUFF =====================================
        void worker_DoWork(object sender, DoWorkEventArgs e)
        {
            var worker = (BackgroundWorker)sender;

            if (((sWorkerParms)e.Argument).what == 1) _trainClient(((sWorkerParms)e.Argument).simulationId, ((sWorkerParms)e.Argument).clientXML, ((sWorkerParms)e.Argument).dataShapeXML, ((sWorkerParms)e.Argument).dataSetXML, ((sWorkerParms)e.Argument).engineXML, worker.ReportProgress);
            if (((sWorkerParms)e.Argument).what == 2) _inferClient(((sWorkerParms)e.Argument).simulationId, ((sWorkerParms)e.Argument).clientXML, ((sWorkerParms)e.Argument).dataShapeXML, ((sWorkerParms)e.Argument).dataSetXML, ((sWorkerParms)e.Argument).engineXML, ((sWorkerParms)e.Argument).savedEnginePid, worker.ReportProgress);
            if (((sWorkerParms)e.Argument).what == 3) _bothClient(((sWorkerParms)e.Argument).simulationId, ((sWorkerParms)e.Argument).clientXML, ((sWorkerParms)e.Argument).dataShapeXML, ((sWorkerParms)e.Argument).dataSetXML, ((sWorkerParms)e.Argument).engineXML, worker.ReportProgress);

        }
        void worker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            //pbCalculationProgress.Value = e.ProgressPercentage;
            pbCalculationProgress.Value =e.ProgressPercentage;
            tbProgress.Text = tbProgress.Text + e.UserState;

            //if (e.UserState != null)
            // lbResults.Items.Add(e.UserState);
        }
        void worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            MessageBox.Show("Numbers between 0 and 10000 divisible by 7: " + e.Result);
        }
        //===================================================================================
    }
}

