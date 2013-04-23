CREATE TABLE Users (
    user_id INT PRIMARY KEY,
    first_name VARCHAR(50),
    last_name VARCHAR(50),
    email VARCHAR(50)
);
CREATE TABLE Hubs (
    hub_id INT PRIMARY KEY,
    user_id INT NOT NULL,
    name VARCHAR(50)
);
CREATE TABLE Nodes (
    node_id INT PRIMARY KEY,
    hub_id INT NOT NULL,
    name VARCHAR(50)
);
CREATE TABLE Node_Data (
    node_id INT NOT NULL,
    data_type VARCHAR(10) NOT NULL,
    data_point VARCHAR(50) NOT NULL,
    time TIMESTAMP NOT NULL,
);
