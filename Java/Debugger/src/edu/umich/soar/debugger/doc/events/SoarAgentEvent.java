/********************************************************************************************
 *
 * SoarAgentEvent.java
 *
 * Description:
 *
 * Created on 	Jan 31, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.doc.events;

import sml.Agent;

/************************************************************************
 *
 * Used to report a change to an agent (added, removed etc.)
 *
 ************************************************************************/
public class SoarAgentEvent extends java.util.EventObject {
    private static final long serialVersionUID = -1073490020065002122L;

    public enum EventType {
        AGENT_ADDED,
        AGENT_REMOVED,
        // More generic -- just says the list
        // isn't the same now (use the more
        // specific event if you can)
        LIST_CHANGED;
    }

    private final EventType m_Type;

    private final Agent m_Agent;

    public SoarAgentEvent(Object source, EventType type, Agent agent) {
        super(source);

        m_Type = type;
        m_Agent = agent;
    }

    public boolean isAgentAdded() {
        return m_Type.equals(EventType.AGENT_ADDED);
    }

    public boolean isAgentRemoved() {
        return m_Type.equals(EventType.AGENT_REMOVED);
    }

    public boolean isAgentListChanged() {
        return isAgentAdded() || isAgentRemoved() || m_Type.equals(EventType.LIST_CHANGED);
    }

    public Agent getAgent() {
        return m_Agent;
    }
}
