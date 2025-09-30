#!/bin/bash

sudo mkdir -p /etc/ssl/mycerts
sudo cp server.crt /etc/ssl/mycerts/
sudo cp server.key /etc/ssl/mycerts/
sudo chmod 600 /etc/ssl/mycerts/server.key

echo "server {
    listen 443 ssl;
    server_name _;

    root /var/www/html;
    index index.html;

    ssl_certificate     /etc/ssl/mycerts/server.crt;
    ssl_certificate_key /etc/ssl/mycerts/server.key;
}
" > /etc/nginx/sites-available/default

echo "<h1>HTTPS works</h1>" > /var/www/html/index.html
sudo nginx -t
sudo systemctl restart nginx