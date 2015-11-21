
--
-- Database: statistics.db
--

CREATE TABLE stats_network
(
	id INTEGER PRIMARY KEY,
	lastupdateon TEXT NOT NULL,

	receivedbytes INTEGER DEFAULT NULL,
	sentbytes INTEGER DEFAULT NULL,
	
);

CREATE TABLE stats_participants
(
	id INTEGER PRIMARY KEY,
	userid TEXT DEFAULT NULL,	-- Local generated unique ID.
	
	ip TEXT NOT NULL,
	name TEXT NOT NULL,
	connectedon TEXT NOT NULL,
	disconnectedon TEXT DEFAULT NULL
);