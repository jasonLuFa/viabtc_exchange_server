-- Add initial balance for user 1
INSERT INTO slice_balance (user_id, asset, t, balance) VALUES 
(1, 'BTC', 1, 1000.0000000000000000),
(1, 'ETH', 1, 1000.0000000000000000),
(1, 'BCH', 1, 10000.0000000000000000);

-- Add initial balance for user 2
INSERT INTO slice_balance (user_id, asset, t, balance) VALUES 
(2, 'BTC', 2, 2000.0000000000000000),
(2, 'ETH', 2, 20000.0000000000000000),
(2, 'BCH', 2, 200000.0000000000000000);
