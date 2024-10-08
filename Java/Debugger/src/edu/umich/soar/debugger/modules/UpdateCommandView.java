/********************************************************************************************
 *
 * UpdateCommandView.java
 *
 * Description:
 *
 * Created on 	Mar 18, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.modules;

import sml.Agent;

/************************************************************************
 *
 * This version of the BaseCommandView updates at the end of each run and clears
 * each time a command is issued.
 *
 ************************************************************************/
public class UpdateCommandView extends AbstractSingleTextView
{
    public UpdateCommandView()
    {
        m_ClearComboEachCommand = false;
        m_ClearEachCommand = true;
        m_UpdateOnStop = true;
    }

    /********************************************************************************************
     *
     * This "base name" is used to generate a unique name for the window. For
     * example, returning a base name of "trace" would lead to windows named
     * "trace1", "trace2" etc.
     *
     ********************************************************************************************/
    @Override
    public String getModuleBaseName()
    {
        return "update";
    }

    @Override
    public String toString() {
        Agent agent = getAgentFocus();
        return "UpdateCommandView, command=" + (m_CurrentCommand == null ? "null" : m_CurrentCommand) +
            ", agent=" + (agent == null ? "null" : agent.GetAgentName());
    }
}
