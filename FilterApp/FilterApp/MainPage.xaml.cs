using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace FilterApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        const string restrictionPath = "restrictions.txt";
        const string displayPath = "displaypath.txt";
        const string noaccess = "NO ACCESS";
        const string readOnly = "READ ONLY";
        const string writeOnly = "WRITE ONLY";
        const string completeAccess = "COMPLETE ACCESS";

        Windows.Storage.StorageFolder storageFolder =
            Windows.Storage.ApplicationData.Current.LocalFolder;

        public MainPage()
        {
            this.InitializeComponent();
            comboBox.Items.Add(noaccess);
            comboBox.Items.Add(readOnly);
            comboBox.Items.Add(writeOnly);
            comboBox.Items.Add(completeAccess);
            comboBox.SelectedItem = noaccess;
            update();
        }
        private async void update()
        {
            Windows.Storage.StorageFile dispFile =
               await storageFolder.CreateFileAsync(displayPath,
                   Windows.Storage.CreationCollisionOption.OpenIfExists);

            IList<string> read = await Windows.Storage.FileIO.ReadLinesAsync(dispFile);
            for (int i = 0; i < read.Count; i++)
            {
                if (listbox.Items.Count <= i)
                {
                    listbox.Items.Add(read[i]);
                }
                else if (read[i] != listbox.Items[i])
                {
                    listbox.Items[i] = read[i];
                }
            }
        }
        private async void button_Click(object sender, RoutedEventArgs e)
        {
            Windows.Storage.StorageFile restFile =
                await storageFolder.CreateFileAsync(restrictionPath,
                    Windows.Storage.CreationCollisionOption.OpenIfExists);


            Windows.Storage.StorageFile dispFile =
                await storageFolder.CreateFileAsync(displayPath,
                    Windows.Storage.CreationCollisionOption.OpenIfExists);

            string restStream = filename_textbox.Text;
            string selected = comboBox.SelectedItem.ToString();
            string displayStream = filename_textbox.Text;


            byte perm = 0;
            switch (selected)
            {
                case (noaccess):
                    perm = 0;
                    break;
                case (readOnly):
                    perm = 1;
                    break;
                case (writeOnly):
                    perm = 5;
                    break;
                case (completeAccess):
                    perm = 7;
                    break;
                default:
                    perm = 0;
                    break;
            }

            // :perm:filepath;
            restStream = ":" + perm + ":" + restStream + ";\r\n";
            displayStream = "\"" + displayStream + "\"" + " => PERMISSION: " + selected;
            listbox.Items.Add(displayStream);
            displayStream += "\r\n";
            System.Diagnostics.Debug.Write(restStream);
            await Windows.Storage.FileIO.AppendTextAsync(restFile, restStream);
            await Windows.Storage.FileIO.AppendTextAsync(dispFile, displayStream);
            listbox.Items.Add(displayStream);

        }

        private void textBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void comboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }

        private void textBlock_SelectionChanged(object sender, RoutedEventArgs e)
        {

        }

        private void ListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            
        }

        private async void chooseFileButton_Click(object sender, RoutedEventArgs e)
        {
            var picker = new Windows.Storage.Pickers.FileOpenPicker();
            picker.ViewMode = Windows.Storage.Pickers.PickerViewMode.Thumbnail;
            picker.FileTypeFilter.Add("*");
            Windows.Storage.StorageFile file = await picker.PickSingleFileAsync();
            if (file != null)
            {
                filename_textbox.Text = file.Path;
            }
            else
            {
            }
        }

        private async void chooseFolderButton_Click(object sender, RoutedEventArgs e)
        {
            var folderPicker = new Windows.Storage.Pickers.FolderPicker();
            folderPicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.Desktop;
            folderPicker.FileTypeFilter.Add("*");

            Windows.Storage.StorageFolder folder = await folderPicker.PickSingleFolderAsync();
            if (folder != null)
            {
                // Application now has read/write access to all contents in the picked folder
                // (including other sub-folder contents)
                Windows.Storage.AccessCache.StorageApplicationPermissions.
                FutureAccessList.AddOrReplace("PickedFolderToken", folder);
                System.Diagnostics.Debug.Write(folder.Path);
                filename_textbox.Text = folder.Path;
            }
            else
            {
            }
        }

        private async void refresh_button_click(object sender, RoutedEventArgs e)
        {
            Windows.Storage.StorageFile restFile =
                await storageFolder.CreateFileAsync(restrictionPath,
                    Windows.Storage.CreationCollisionOption.OpenIfExists);

            Windows.Storage.StorageFile dispFile =
                await storageFolder.CreateFileAsync(displayPath,
                    Windows.Storage.CreationCollisionOption.OpenIfExists);

            IList<string> read = await Windows.Storage.FileIO.ReadLinesAsync(restFile);
            IList<string> dispread = await Windows.Storage.FileIO.ReadLinesAsync(dispFile);

            string f;
            bool match = false;
            IList<string> q = new List<string>();
            for (int i = 0; i < read.Count; i++)
            {
                f = read[i].Substring(2, read[i].Length - 3);

                if (match == false)
                {
                    string perm = noaccess;
                    switch (f[2])
                    {
                        case ('0'):
                            perm = noaccess;
                            break;
                        case ('1'):
                            perm = readOnly;
                            break;
                        case ('5'):
                            perm = writeOnly;
                            break;
                        case ('7'):
                            perm = completeAccess;
                            break;
                        default:
                            perm = noaccess;
                            break;
                    }
                    f = "\"" + f + "\"" + " => PERMISSION: " + perm;
                    
                    q.Add(f);
                    
                }

            }
            await Windows.Storage.FileIO.WriteLinesAsync(dispFile, q);
            update();
        }

        private async void Unblock_Click(object sender, RoutedEventArgs e)
        {
            int index = listbox.SelectedIndex;
            if (index != -1)
            {
               Windows.Storage.StorageFile restFile =
                await storageFolder.CreateFileAsync(restrictionPath,
                    Windows.Storage.CreationCollisionOption.OpenIfExists);

                Windows.Storage.StorageFile dispFile =
                    await storageFolder.CreateFileAsync(displayPath,
                        Windows.Storage.CreationCollisionOption.OpenIfExists);

                IList<string> read = await Windows.Storage.FileIO.ReadLinesAsync(restFile);
                IList<string> dispread = await Windows.Storage.FileIO.ReadLinesAsync(dispFile);
                read.RemoveAt(index);
                dispread.RemoveAt(index);
                listbox.Items.RemoveAt(index);
                await Windows.Storage.FileIO.WriteLinesAsync(restFile, read);
                await Windows.Storage.FileIO.WriteLinesAsync(dispFile, dispread);
            }
        }
    }
}
