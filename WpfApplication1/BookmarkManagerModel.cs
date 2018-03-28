using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace BookMark
{
    public class BookmarkManagerModel
    {
        public ObservableCollection<Bookmark> bookmarks { get; set; }
        public int curBookmarkLocation { get; set; }
    }

    public class Bookmark : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        public void OnPropertyChanged(string propertyName = "")
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public int Location { get; set; }

        private string _strName;
        public string strName
        {
            get { return _strName; }
            set 
            {
                _strName = value;
                OnPropertyChanged("strName");
            }
        }

        private string _strComment;
        public String strComment
        {
            get { return _strComment; }
            set
            {
                _strComment = value;
                OnPropertyChanged("strComment");
            }
        }

        private string _strTimestamp;
        public String strTimestamp 
        {
            get { return _strTimestamp; }
            set
            {
                _strTimestamp = value;
                OnPropertyChanged("strTimestamp");           
            }
        }

        private string _strPort;
        public String strPort 
        {
            get { return _strPort; }
            set
            {
                _strPort = value;
                OnPropertyChanged("strPort");
            }
        }
    }
}
