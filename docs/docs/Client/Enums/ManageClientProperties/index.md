# ManageClientProperties

## Preface

These indexs are for managing client(s) and are used to store temporarily cookies or replybacks mainly managerequest() being cookie(s) and managereplies() retrieving replybacks with manage() using the replybacks.  

Fields in managerequest() should be stack allocated though not required.  

Fileds in managereplies() may return NULL, and should be freed if non NULL.  