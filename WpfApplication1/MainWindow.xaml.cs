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

using System.Collections.ObjectModel;
using System.ComponentModel;

namespace WpfApplication1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
    public class CaptureInfo_Wpf
    {
        public String sTraceDate { get; set; }
        public String sStopTime { get; set; }
        public String ProtocolAsString { get; set; }
    };

    public partial class MainWindow : Window
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }

        private String _CurrentDomainName;
        public String CurrentDomainName
        {
            get { return _CurrentDomainName; }
            set
            {
                _CurrentDomainName = value;
                //MessageBox.Show(_CurrentDomainName);
                OnPropertyChanged("CurrentDomainName");
            }
        }

        public int NumberOfChannels { get; set; }
        public CaptureInfo_Wpf[] CaptureInfos { get; set; }

        public String getTraceInfoDlgInfo()
        {
            return "TraceInfoDlgInfo";
        }

        public MainWindow()
        {
            InitializeComponent();

            this.DataContext = this;
        }
    }

}
