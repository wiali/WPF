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

namespace WpfApplication1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
    public class CaptureInfo_Wpf
    {
        public String sTraceDate;
	    public String sStopTime;
	    public String ProtocolAsString;
    };

    public partial class MainWindow : Window
    {
        public String CurrentDomainName { get; set; }
        public int NumberOfChannels { get; set; }
        public CaptureInfo_Wpf[] CaptureInfos { get; set; }

        public String getTraceInfoDlgInfo()
        {
            return "TraceInfoDlgInfo";
        }

        public MainWindow()
        {
            InitializeComponent();
        }
    }
}
