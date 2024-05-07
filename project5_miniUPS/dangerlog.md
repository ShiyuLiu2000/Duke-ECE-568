# Danger Log

## Security

### 1. Users' Data Exposure

- Users cannot access others’ information.
- **Solution**: Access data through isolated sessions instead of directly searching by users' id.

### 2. Source Code Exposure

- Source code in the production environment may expose sensitive information.
- **Solution**: Set DEBUG to false to close the debug page in the production environment.

### 3. Database Operations

- Ensure atomicity of database operations.
- **Solution**: Convert data operations in views into transactions.

### 4. CSRF Attack

- Potential Cross-Site Request Forgery (CSRF) attack.
- **Solution**: Implement Anti-CSRF token mechanism.

### 5. Malicious Requests

- Verify the source of important CRUD operations to prevent malicious actions.
- **Solution**: Perform delete and update operations using POST forms instead of URLs.

## Bugs

### 1. Non-deterministic Behavior

- Issues with multi-threading causing unpredictable behavior.
- **Race Condition**: Status of trucks and packages changes unexpectedly due to threads' race condition.
- **Front-end Socket Communication**: Ensure proper configuration for socket communication between front-end and back-end within Docker.
- **Primary Key (PK) Violence of Item**: Address issues related to primary key conflicts.

## Robustness

### 1. Error Handling

- Implement try-catch blocks to prevent server crashes.

### 2. Handling External Failures

- Handle failures related to building sockets, connecting to web servers, and data transmission.

### 3. Concurrency Control

- **Database Lock**: Address concurrency issues in the database by adding locks.
- **Socket Lock**: Prevent concurrency problems in socket communication by adding locks.

### 4. Thread Management

- Consider using thread pool instead of threading to prevent buffer overflow.

### 5. Resilience to External Factors

- Handle cases such as network flakiness and duplicate responses from external systems.

### 6. Access Control

- Ensure users cannot access others' data without proper authentication.

### 7. Data Integrity

- Address errors related to primary key conflicts and missing UPS account information.

### 8. Graceful Shutdown

- Implement graceful exit mechanism to handle server failures and reconnections to external systems.


