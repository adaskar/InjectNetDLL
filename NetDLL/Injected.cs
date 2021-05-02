using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NetDLL
{
    public class Injected
    {
        private static void btnSayHiFromInjected_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Hello world from .Injected DLL");
        }

        private static void RemoveNetAppEventHandler(Button btnSayHi)
        {
            object current = null;
            EventHandlerList events = null;


            events = (EventHandlerList)typeof(Component)
                .GetProperty("Events", BindingFlags.NonPublic | BindingFlags.Instance)
                .GetValue(btnSayHi, null);

            current = events?.GetType()
                .GetField("head", BindingFlags.NonPublic | BindingFlags.Instance)
                .GetValue(events);

            while (current != null)
            {
                object handler = current.GetType()
                    .GetField("handler", BindingFlags.NonPublic | BindingFlags.Instance)
                    .GetValue(current);

                if (handler is EventHandler eh &&
                    eh.Method.Name == "btnSayHi_Click")
                {
                    object key = current.GetType()
                        .GetField("key", BindingFlags.NonPublic | BindingFlags.Instance)
                        .GetValue(current);

                    events.RemoveHandler(key, eh.GetInvocationList()[0]);
                    break;
                }

                current = current.GetType()
                    .GetField("next", BindingFlags.NonPublic | BindingFlags.Instance)
                    .GetValue(current);
            }
        }

        public static int InjectedMethod(string pwzArgument)
        {
            IntPtr ptrMainWndHandle = IntPtr.Zero;

            ptrMainWndHandle = Process.GetCurrentProcess().MainWindowHandle;
            if (ptrMainWndHandle == IntPtr.Zero ||
                !(Form.FromHandle(ptrMainWndHandle) is Form form))
            {
                MessageBox.Show("Error", "Could not get main window", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return 1;
            }

            form.Invoke(new MethodInvoker(() =>
            {
                form.Text = ".NET DLL Injected";

                RemoveNetAppEventHandler(form.Controls["btnSayHi"] as Button);
                (form.Controls["btnSayHi"] as Button).Click += btnSayHiFromInjected_Click;
            }));

            return 0;
        }
    }
}
