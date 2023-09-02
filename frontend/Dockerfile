# syntax=docker/dockerfile:1

#* frontend-env stage: install dependencies
# Minimum NodeJS Version: 16.14
FROM node:20.5.1-buster AS frontend-env
ENV HOST=0.0.0.0
WORKDIR /usr/src/app

RUN npm install -g pnpm@8.7.0

COPY pnpm-lock.yaml ./
COPY package.json ./
RUN pnpm install

COPY svelte.config.js ./
COPY vite.config.ts ./
COPY tsconfig.json ./
COPY ./static /usr/src/app/static
COPY ./src /usr/src/app/src