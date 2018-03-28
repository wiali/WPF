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

namespace BookMark
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class BookmarkManagerDlg : Window
    {
        public BookmarkManagerModel bookmarkModel;

        public BookmarkManagerDlg()
        {
            InitializeComponent();

            // To be replaced
            bookmarkModel = new BookmarkManagerModel();
            bookmarkModel.bookmarks = new ObservableCollection<Bookmark>();
            for (int i = 0; i < 10; i++)
            {
                Bookmark bookmarkItem = new Bookmark();
                bookmarkItem.Location = i;
                bookmarkItem.strName = "BookMark " + i.ToString();
                bookmarkItem.strComment = "Commment " + i.ToString();
                bookmarkItem.strPort = String.Format("Port(1,{0})", i);
                bookmarkItem.strTimestamp = String.Format("Timestamp {0}", i);
                bookmarkModel.bookmarks.Add(bookmarkItem);
            }
            this.DataContext = bookmarkModel;

            BookmarkList.SelectedIndex = 0;
        }

        private void MenuExportBookmarks_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Export Bookmarks.");
        }

        private void MenuImportBookmarks_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Import Bookmarks.");
        }     

        private void BookmarkList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (BookmarkList.SelectedItem == null)
            {
                Name.Text = "";
                Comment.Text = "";
                Timestamp.Text = "";
                Port.Text = "";
                return;
            }
            Bookmark selBookmark = BookmarkList.SelectedItem as Bookmark;
            Name.Text = selBookmark.strName;
            Comment.Text = selBookmark.strComment;
            Timestamp.Text = selBookmark.strTimestamp;
            Port.Text = selBookmark.strPort;
            bookmarkModel.curBookmarkLocation = selBookmark.Location;
        }
     
        private void Name_LostFocus(object sender, RoutedEventArgs e)
        {
            foreach (Bookmark bkItem in bookmarkModel.bookmarks)
            {
                if (bkItem.Location == bookmarkModel.curBookmarkLocation)
                {
                    bkItem.strName = Name.Text;
                    break;
                }
            }
        }

        private void Comment_LostFocus(object sender, RoutedEventArgs e)
        {
            foreach (Bookmark bkItem in bookmarkModel.bookmarks)
            {
                if (bkItem.Location == bookmarkModel.curBookmarkLocation)
                {
                    bkItem.strComment = Comment.Text;
                    break;
                }
            }
        }

        private void Delete_Click(object sender, RoutedEventArgs e)
        {
            int index = BookmarkList.SelectedIndex;
            for (int i = 0; i < bookmarkModel.bookmarks.Count; i++)
            {
                if (bookmarkModel.bookmarks[i].Location == bookmarkModel.curBookmarkLocation)
                {
                    bookmarkModel.bookmarks.RemoveAt(i);
                    i--;
                }
            }

            // Select next bookmark
            if (index < bookmarkModel.bookmarks.Count)
                BookmarkList.SelectedIndex = index;
            else if (index == bookmarkModel.bookmarks.Count)
                BookmarkList.SelectedIndex = index - 1;
        }

        private void DeleteAll_Click(object sender, RoutedEventArgs e)
        {
            bookmarkModel.bookmarks.Clear();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
