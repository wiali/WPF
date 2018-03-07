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
        private String _currentDomainName;
        private int _numberOfChannels;
        private CaptureInfo_Wpf[] _captureInfos;
        public void setCurrentDomainName(String name)
        {
            _currentDomainName = name;
        }

        public void setNumberOfChannels(int num)
        {
            _numberOfChannels = num;
        }

        public void setCaptureInfos(CaptureInfo_Wpf[] arrCaptures)
        {
            _captureInfos = arrCaptures;
        }

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
