CREATE TABLE Users (
    user_id INT PRIMARY KEY, --Might just be Facebook provided ID
    email VARCHAR(50) NOT NULL,
    first_name VARCHAR(50),
    last_name VARCHAR(50),
    api_key VARCHAR(50),
    pin INT
);
CREATE TABLE Hubs (
    hub_id VARCHAR(30) PRIMARY KEY,
    user_id INT NOT NULL,
    pan_id INT, --XBee
    name VARCHAR(50) NOT NULL,
    location VARCHAR(50)
);
CREATE TABLE Nodes (
    node_id INT PRIMARY KEY,
    hub_id INT NOT NULL,
    address VARCHAR(16), --hex
    name VARCHAR(50) NOT NULL,
    type VARCHAR(10) NOT NULL
);
CREATE TABLE Pins (
    pin_id INT PRIMARY KEY,
    node_id INT NOT NULL,
    data_type VARCHAR(10) NOT NULL
);
CREATE TABLE Node_Data (
    pin_id INT NOT NULL,
    pin_value VARCHAR(50) NOT NULL,
    time TIMESTAMP NOT NULL
);
