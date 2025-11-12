# Use .NET SDK as base image
FROM mcr.microsoft.com/dotnet/sdk:5.0 AS build

# Set working directory
WORKDIR /app

# Copy the entire solution
COPY . .

# Install Python dependencies
WORKDIR /app/viewer
RUN dotnet restore
RUN dotnet build -c Release

# Build the C# projects
WORKDIR /app/simulation
RUN dotnet restore
RUN dotnet build -c Release

# Create startup script
WORKDIR /app
COPY start-services.sh .
RUN chmod +x start-services.sh

# Set the entry point
ENTRYPOINT ["./start-services.sh"]