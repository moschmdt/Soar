/********************************************************************************************
 *
 * SWTApplication.java
 *
 * Description:
 *
 * Created on 	Mar 2, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger;

import edu.umich.soar.debugger.doc.Document;
import org.apache.commons.cli.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import sml.Agent;
import sml.Kernel;

import java.util.ArrayList;
import java.util.List;

/************************************************************************
 *
 * SWT based application (we used to have a Swing version too)
 *
 ************************************************************************/
public class SWTApplication
{
    private static final HelpFormatter HELP_FORMATTER = new HelpFormatter();

    public Options getCommandLineOptions() {
        Options options = new Options();
        options.addOption("remote", false, "Open a remote connection to a running kernel. Connect to localhost on Soar's default port unless otherwise specified. If no remote options are set, start a local kernel.");
        options.addOption("ip", true, "Open a remote connection to a running kernel using at this IP address.");
        options.addOption("port", true, "Open a remote connection to a running kernel using at this port.");
        options.addOption("agent", true, "On a remote connection, select the specified agent as the initial agent; on a local connection, assign the specified name to the initial agent.");
        options.addOption("source", true, "Source the specified .soar file on launch (only valid for local kernel).");
        options.addOption("quitonfinish", false, "When combined with -source, exit the debugger after sourcing the specified file.");
        options.addOption("listen", true, "Listen on the specified port for remote connections (only valid for a local kernel).");
        options.addOption("maximize", false, "Start with a maximized window.");
        options.addOption("width", true, "Start with this window width.");
        options.addOption("height", true, "Start with this window height.");
        options.addOption("x", true, "Start with this window x position.");
        options.addOption("y", true, "Start with this window y position.");
        options.addOption("layout", true, "Path to a layout XML file to initialize the debugger window with. A default is loaded if not specified, and this default is updated to the current settings whenever the debugger is closed.");
        options.addOption("help", false, "Print this help message and exit.");
        return options;
    }

    public void startApp(String[] args, Display display) {

        List<String> startupErrors = new ArrayList<>();
        boolean hasCommandLineError = false;

        Options cmdConfig = getCommandLineOptions();
        CommandLineParser parser = new DefaultParser();
        CommandLine cmd;
        try {
            cmd = parser.parse(cmdConfig, args);
        } catch (ParseException e) {
            System.err.println(e.getMessage());
            startupErrors.add(e.getMessage());
            hasCommandLineError = true;
            cmd = new CommandLine() {
            };
        }

        if(cmd.hasOption("help")) {
            printHelp(cmdConfig);
            return;
        }

        // The document manages the Soar process
        Document m_Document = new Document();

        // Check for command line options
        boolean remote = cmd.hasOption("remote");
        String ip = cmd.getOptionValue("ip");
        String port = cmd.getOptionValue("port");
        String agentName = cmd.getOptionValue("agent");
        String source = cmd.getOptionValue("source");
        boolean quitOnFinish = cmd.hasOption("quitonfinish");
        String listen = cmd.getOptionValue("listen");
        String layout = cmd.getOptionValue("layout");

        // quitOnFinish is only valid if sourcing a file
        if (source == null)
            quitOnFinish = false;

        boolean maximize = cmd.hasOption("maximize");

        // Remote args
        if (ip != null || port != null)
        {
            remote = true;
        }

        int portNumber = Kernel.GetDefaultPort();
        if (port != null)
        {
            try
            {
                portNumber = Integer.parseInt(port);
            }
            catch (NumberFormatException e)
            {
                String errorMsg = "Passed invalid port value " + port;
                System.err.println(errorMsg);
                startupErrors.add(errorMsg);
                hasCommandLineError = true;
            }
        }

        int listenPort = Kernel.GetDefaultPort();
        if (listen != null)
        {
            try
            {
                listenPort = Integer.parseInt(listen);
            }
            catch (NumberFormatException e) {
                String errorMsg = "Passed invalid listen value " + listen;
                startupErrors.add(errorMsg);
                System.err.println(errorMsg);
                hasCommandLineError = true;
            }
        }

        // Window size args
        // We'll override the existing app property values
        if (maximize)
            m_Document.getAppProperties()
                    .setAppProperty("Window.Max", maximize);

        String[] numericLayoutOptions = new String[] { "-width", "-height", "-x", "-y" };
        String[] props = new String[] { "Window.width", "Window.height",
                "Window.x", "Window.y" };

        for (int i = 0; i < numericLayoutOptions.length; i++)
        {
            String value = cmd.getOptionValue(numericLayoutOptions[i]);
            if (value != null)
            {
                try
                {
                    int val = Integer.parseInt(value);
                    m_Document.getAppProperties().setAppProperty(props[i], val);

                    // If provide any window information we'll start
                    // not-maximized
                    m_Document.getAppProperties().setAppProperty("Window.Max",
                            false);
                }
                catch (NumberFormatException e)
                {
                    String errorMsg = "Passed invalid value " + value
                        + " for option " + numericLayoutOptions[i];
                    System.err.println(errorMsg);
                    startupErrors.add(errorMsg);
                    hasCommandLineError = true;
                }
            }
        }

        boolean owned = true;
        if (display == null)
        {
            owned = false;
            Display.setAppName("Soar Debugger");
            display = new Display();
        }
        Shell shell = new Shell(display);

        Image small = new Image(display, SWTApplication.class
                .getResourceAsStream("/images/debugger16.png"));
        Image large = new Image(display, SWTApplication.class
                .getResourceAsStream("/images/debugger.png"));
        shell.setImages(new Image[] { small, large });

        // We default to showing the contents of the clipboard in the search
        // dialog
        // so clear it when the app launches, so we don't get random junk in
        // there.
        // Once the app is going, whatever the user is copying around is a
        // reasonable
        // thing to start from, but things from before are presumably unrelated.
        // This currently fails on Linux version of SWT
        // clearClipboard() ;

        final MainFrame frame = new MainFrame(shell, m_Document);
        frame.initComponents(layout);

        // We wait until we have a frame up before starting the kernel
        // so it's just as if the user chose to do this manually
        // (saves some special case logic in the frame)
        if (!remote)
        {
            Agent agent = m_Document.startLocalKernel(listenPort, agentName,
                    source, quitOnFinish);
            frame.setAgentFocus(agent);
        }
        else
        {
            // Start a remote connection
            try
            {
                System.err.println("Starting remote connection...");
                m_Document.remoteConnect(ip, portNumber, agentName);
            }
            catch (Exception e)
            {
                e.printStackTrace();
                startupErrors.add(e.getMessage());
            }
        }

        if (hasCommandLineError) {
            printHelp(cmdConfig);
        }

        shell.open();

        // We show all of the accumulated errors upon opening the application
        if (!startupErrors.isEmpty())
        {
            String errorMessage = String.join("\n", startupErrors);
            MainFrame.ShowMessageBox(shell, "Command line argument errors", errorMessage, SWT.ERROR);
        }
        if (!owned)
        {
            m_Document.pumpMessagesTillClosed(display);

            display.dispose();
        }
    }

    private static void printHelp(Options cmdConfig) {
        HELP_FORMATTER.printHelp("SoarJavaDebugger", cmdConfig);
    }
}
