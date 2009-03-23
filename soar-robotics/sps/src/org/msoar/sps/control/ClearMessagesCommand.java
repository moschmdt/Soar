/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class ClearMessagesCommand implements Command {
	private static final Logger logger = Logger.getLogger(ClearMessagesCommand.class);
	static final String NAME = "clear-messages";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		inputLink.clearMessages();
		logger.info(NAME + ":");
		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return false;
	}

	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}
}