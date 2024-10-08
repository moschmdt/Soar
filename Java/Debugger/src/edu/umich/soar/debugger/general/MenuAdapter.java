/********************************************************************************************
 *
 * MenuAdapter.java
 *
 * Created on 	Nov 7, 2003
 *
 * @author 		Doug
 * @version
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.general;

import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;

/********************************************************************************************
 *
 * Simple adapter class for menu events.
 *
 ********************************************************************************************/
public class MenuAdapter implements MenuListener
{
    @Override
    public void menuCanceled(MenuEvent e)
    {
    }

    @Override
    public void menuDeselected(MenuEvent e)
    {
    }

    @Override
    public void menuSelected(MenuEvent e)
    {
    }
}
